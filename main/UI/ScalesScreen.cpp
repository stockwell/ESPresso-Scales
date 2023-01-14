#include "ScalesScreen.hpp"

#include "m5core2_axp192.h"

ScalesScreen::ScalesScreen(ScalesEventLoop* scales)
	: m_scalesEventLoop(scales)
{

}

static void tare_btn_cb(lv_obj_t* obj, lv_event_t e)
{
	if (e != LV_EVENT_PRESSED)
		return;

	auto* scalesEventLoop = static_cast<ScalesEventLoop*>(lv_obj_get_user_data(obj));
	scalesEventLoop->tare();
}

static void shutdown_btn_cb(lv_obj_t* obj, lv_event_t e)
{
	if (e != LV_EVENT_PRESSED)
		return;

	auto* scalesScreen = static_cast<ScalesScreen*>(lv_obj_get_user_data(obj));
	scalesScreen->shutdown();
}

static void weightUpdateTask(lv_task_t* task)
{
	auto* scalesScreen = static_cast<ScalesScreen*>(task->user_data);
	scalesScreen->updateWeight();
}

static void batteryUpdateTask(lv_task_t* task)
{
	auto* scalesScreen = static_cast<ScalesScreen*>(task->user_data);
	scalesScreen->updateBattery();
}

void ScalesScreen::activate()
{
	m_container = lv_obj_create(lv_scr_act(), nullptr);
	lv_obj_reset_style_list(m_container, LV_OBJ_PART_MAIN);
	lv_obj_set_pos(m_container, 0, 0);
	lv_obj_set_size(m_container, LV_HOR_RES, LV_VER_RES);

	lv_obj_set_style_local_bg_opa(m_container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

	m_weightLabel = lv_label_create(lv_scr_act(), nullptr);
	lv_label_set_text(m_weightLabel, "0.0g");
	lv_obj_set_style_local_text_font(m_weightLabel, 0, 0, &lv_font_montserrat_48);
	lv_obj_align(m_weightLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

	m_tareButton = lv_btn_create(m_container, nullptr);
	lv_obj_set_event_cb(m_tareButton, tare_btn_cb);
	lv_obj_set_user_data(m_tareButton, m_scalesEventLoop);
	lv_obj_align(m_tareButton, nullptr, LV_ALIGN_CENTER, 0, 70);

	lv_obj_t* tareBtnLabel = lv_label_create(m_tareButton, nullptr);
	lv_label_set_text(tareBtnLabel, "Tare");

	m_voltageLabel = lv_label_create(lv_scr_act(), nullptr);
	lv_label_set_text(m_voltageLabel, "---");
	lv_obj_set_style_local_text_font(m_voltageLabel, 0, 0, &lv_font_montserrat_14);
	lv_obj_align(m_voltageLabel, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);

	m_shutdownButton = lv_btn_create(m_container, nullptr);
	lv_obj_set_event_cb(m_shutdownButton, shutdown_btn_cb);
	lv_obj_set_user_data(m_shutdownButton, m_scalesEventLoop);

	lv_obj_t* shutdownButtonLabel = lv_label_create(m_shutdownButton, nullptr);
	lv_label_set_text(shutdownButtonLabel, LV_SYMBOL_POWER);

	lv_obj_set_size(m_shutdownButton, 40, 40);
	lv_obj_set_style_local_radius(m_shutdownButton, 0, 0, 40);
	lv_obj_align(m_shutdownButton, nullptr, LV_ALIGN_IN_TOP_RIGHT, -10, 10);

	m_batteryTask = lv_task_create(weightUpdateTask, 50, LV_TASK_PRIO_HIGHEST, (void*)this);
	m_weightTask = lv_task_create(batteryUpdateTask, 1000, LV_TASK_PRIO_LOWEST, (void*)this);
}

void ScalesScreen::updateWeight()
{
	auto weight = m_scalesEventLoop->getWeight();

	if (weight < 0)
		lv_label_set_text(m_weightLabel, "----");
	else
		lv_label_set_text_fmt(m_weightLabel, "%0.01fg", weight);

	lv_obj_align(m_weightLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
}

void ScalesScreen::updateBattery()
{
	float voltage = 0.0f;
	if (m5core2_axp_read(AXP192_BATTERY_VOLTAGE, &voltage) == ESP_OK)
		lv_label_set_text_fmt(m_voltageLabel, "%0.02fv", voltage);
}

void ScalesScreen::shutdown()
{
	uint8_t buffer;
	m5core2_axp_read_reg(AXP192_SHUTDOWN_BATTERY_CHGLED_CONTROL, &buffer);
	m5core2_axp_write_reg(AXP192_SHUTDOWN_BATTERY_CHGLED_CONTROL, buffer | 0x80);
}
