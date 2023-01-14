#include "Ui.hpp"

#include "lvgl.h"
#include "lvgl_helpers.h"

namespace
{
	enum Events : uint8_t
	{
		TickTimerElapsed,
		UpdateStarted,
		UpdateStopped,
	};

	constexpr auto kTickPeriod = 10;
}

static void lv_tick_task(void*)
{
	lv_tick_inc(kTickPeriod);
}

static void lv_port_init()
{
	lv_init();

	// Initialize SPI
	lvgl_driver_init();

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;

	auto* buf1 = static_cast<lv_color_t*>(heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA));
	assert(buf1 != nullptr);

	auto* buf2 = static_cast<lv_color_t*>(heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA));
	assert(buf2 != nullptr);

	static lv_disp_buf_t disp_buf;

	uint32_t size_in_px = DISP_BUF_SIZE;

	// Initialize the working buffer depending on the selected display.
	lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	// Initialise touch
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.read_cb = touch_driver_read;
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	lv_indev_drv_register(&indev_drv);
}

ESPressoScalesUI::ESPressoScalesUI(ScalesEventLoop* scales)
	: EventLoop("UIEventLoop", 4096)
	, m_loadingScreen()
	, m_scalesScreen(scales)
	, m_scales(scales)
{
	m_timer = std::make_unique<Timer>(10, [this]() {
		eventPost(Events::TickTimerElapsed);
	}, false);

	lv_port_init();

	// Create and start a periodic timer interrupt to call lv_tick_inc
	esp_timer_create_args_t periodic_timer_args = {};
	periodic_timer_args.callback = &lv_tick_task;
	periodic_timer_args.name = "periodic_gui";

	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, kTickPeriod * 1000));

	lv_theme_t* th = lv_theme_material_init(lv_color_hex(0x04ADE6), lv_color_hex(0x0000FF),
		LV_THEME_MATERIAL_FLAG_LIGHT,
		&lv_font_montserrat_16, &lv_font_montserrat_16, &lv_font_montserrat_16, &lv_font_montserrat_16);

	lv_theme_set_act(th);

	m_loadingScreen.init();
}

void ESPressoScalesUI::start()
{
	m_timer->start();
}

void ESPressoScalesUI::eventHandler(int32_t eventId, void* data)
{
	switch (eventId)
	{
	case Events::TickTimerElapsed:
		tick();
		break;
	}
}

void ESPressoScalesUI::tick()
{
	static bool initialised = false;
	if (! initialised && m_scales->ready())
	{
		initialised = true;
		m_loadingScreen.deactivate();
		m_scalesScreen.activate();
	}

	lv_task_handler();

	m_timer->start();
}
