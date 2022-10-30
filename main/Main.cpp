#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_event.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Lib/Wifi.hpp"

#include "i2c_manager.h"
#include "m5core2_axp192.h"

#include "RESTServer/Server.hpp"
#include "ScalesEventLoop.hpp"
#include "Ui.hpp"

#include <memory>

static void mainTask(void*)
{
	auto ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	m5core2_init();

	Wifi::InitWifi(Wifi::WifiMode::STA);
	Wifi::InitMDNS();

	auto scales = new ScalesEventLoop();
	auto ui = new ESPressoScalesUI(scales);
	auto server = new RESTServer(scales);

	scales->init();
	ui->start();

	vTaskDelete(nullptr);
}

extern "C" void app_main()
{
	xTaskCreatePinnedToCore(mainTask, "main", 4096, nullptr, 0, nullptr, 1);
}
