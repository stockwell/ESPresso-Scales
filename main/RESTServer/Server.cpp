#include <cstring>

#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"

#include "RESTServer/Server.hpp"

#include "Updater.hpp"
#include "UpdaterEventLoop.hpp"

namespace
{
	static constexpr auto		kScratchSize = 0x2800;
	struct ServerCtx
	{
		std::array<char, kScratchSize>		buffer;
		ScalesEventLoop*					scalesAPI;
		std::unique_ptr<UpdaterEventLoop>	updaterEventLoop;
	};
}

static esp_err_t validate_post_request(httpd_req_t* req)
{
	int total_len = req->content_len;
	int cur_len = 0;
	auto& buf = ((ServerCtx*)(req->user_ctx))->buffer;
	if (total_len >= buf.size())
	{
		/* Respond with 500 Internal Server Error */
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
		return ESP_FAIL;
	}

	int received = 0;
	while (cur_len < total_len)
	{
		received = httpd_req_recv(req, buf.data() + cur_len, total_len);
		if (received <= 0)
		{
			/* Respond with 500 Internal Server Error */
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
			return ESP_FAIL;
		}
		cur_len += received;
	}
	buf[total_len] = '\0';

	return ESP_OK;
}

static esp_err_t sys_info_get_handler(httpd_req_t *req)
{
	httpd_resp_set_type(req, "application/json");
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "free_heap", esp_get_free_heap_size());
	cJSON_AddNumberToObject(root, "min_free_heap", esp_get_minimum_free_heap_size());

	const char* info = cJSON_Print(root);
	httpd_resp_sendstr(req, info);

	free((void*)info);
	cJSON_Delete(root);
	return ESP_OK;
}

static esp_err_t update_init_post_handler(httpd_req_t* req)
{
	if (auto err = validate_post_request(req); err != ESP_OK)
		return err;

	auto* serverCtx = ((ServerCtx*)(req->user_ctx));

	if (serverCtx->updaterEventLoop == nullptr)
		serverCtx->updaterEventLoop = std::make_unique<UpdaterEventLoop>();

	cJSON* root = cJSON_Parse(serverCtx->buffer.data());
	if (! root)
	{
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, nullptr);
		return ESP_FAIL;
	}

	const auto* url = cJSON_GetObjectItem(root, "URL");
	const auto* uuid = cJSON_GetObjectItem(root, "UUID");
	const auto* size = cJSON_GetObjectItem(root, "filesize");

	if (! url || ! uuid || ! size)
	{
		cJSON_Delete(root);
		httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, nullptr);
		return ESP_FAIL;
	}

	UpdaterEventLoop::UpdateRequest request = {};
	strncpy(request.URL, url->valuestring, sizeof request.URL - 1);
	strncpy(request.UUID, uuid->valuestring, sizeof request.UUID - 1);
	request.filesize = size->valueint;

	if (! serverCtx->updaterEventLoop->initiateUpdate(request))
	{
		cJSON_Delete(root);
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, nullptr);
		return ESP_FAIL;
	}

	// TODO: Generic shutdown notification system
	serverCtx->scalesAPI->shutdown();

	httpd_resp_sendstr(req, "Update Initiated");
	cJSON_Delete(root);

	return ESP_OK;
}

static esp_err_t update_status_get_handler(httpd_req_t* req)
{
	auto* serverCtx = ((ServerCtx*)(req->user_ctx));

	UpdaterEventLoop::UpdateStatus status = {};

	if (serverCtx->updaterEventLoop != nullptr)
		status = serverCtx->updaterEventLoop->getUpdateStatus();

	httpd_resp_set_type(req, "application/json");
	cJSON *root = cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "progress", status.progress);
	cJSON_AddStringToObject(root, "uuid", status.UUID.c_str());

	const char* statusJSON = cJSON_Print(root);
	httpd_resp_sendstr(req, statusJSON);

	free((void*)statusJSON);
	cJSON_Delete(root);

	return ESP_OK;
}

static esp_err_t weight_get_handler(httpd_req_t* req)
{
	auto* serverCtx = ((ServerCtx*)(req->user_ctx));

	httpd_resp_set_type(req, "application/json");
	cJSON *root = cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "weight", serverCtx->scalesAPI->getWeight());

	const char* statusJSON = cJSON_Print(root);
	httpd_resp_sendstr(req, statusJSON);

	free((void*)statusJSON);
	cJSON_Delete(root);

	return ESP_OK;
}

static void registerURIHandler(httpd_handle_t server, const char* uri, http_method method, esp_err_t (*handler)(httpd_req_t *r), ServerCtx* ctx)
{
	httpd_uri_t http_uri =
	{
		.uri = uri,
		.method = method,
		.handler = handler,
		.user_ctx = ctx
	};

	httpd_register_uri_handler(server, &http_uri);
}

RESTServer::RESTServer(ScalesEventLoop* scales)
{
	auto* serverCtx = new ServerCtx;
	serverCtx->scalesAPI = scales;

	httpd_handle_t server = nullptr;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.uri_match_fn = httpd_uri_match_wildcard;
	config.max_uri_handlers = 16;

	printf("RESTServer: Starting server\n");
	if (auto err = httpd_start(&server, &config); err != ESP_OK)
	{
		printf("RESTServer: Start failed!\n");
		return;
	}

	registerURIHandler(server, "/api/v1/sys/info",			HTTP_GET, sys_info_get_handler, serverCtx);
	registerURIHandler(server, "/api/v1/weight",			HTTP_GET, weight_get_handler, serverCtx);

	registerURIHandler(server, "/api/v1/update/initiate",	HTTP_POST, update_init_post_handler, serverCtx);
	registerURIHandler(server, "/api/v1/update/status",		HTTP_GET, update_status_get_handler, serverCtx);
}
