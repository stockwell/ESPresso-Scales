#include "ScalesScreen.hpp"

ScalesScreen::ScalesScreen(ScalesEventLoop* scales)
	: m_scalesEventLoop(scales)
{

}

static void tare_btn_cb(lv_obj_t* obj, lv_event_t e)
{
	if (e != LV_EVENT_CLICKED)
		return;

	auto* scalesEventLoop = static_cast<ScalesEventLoop*>(lv_obj_get_user_data(obj));

	scalesEventLoop->tare();
}

static void weightUpdateTask(lv_task_t* task)
{
	auto* scalesScreen = static_cast<ScalesScreen*>(task->user_data);
	scalesScreen->updateWeight();
}

void ScalesScreen::activate()
{
	m_container = lv_obj_create(lv_scr_act(), nullptr);
	lv_obj_reset_style_list(m_container, LV_OBJ_PART_MAIN);
	lv_obj_set_pos(m_container, 0, 0);
	lv_obj_set_size(m_container, LV_HOR_RES, LV_VER_RES);

	lv_obj_set_style_local_bg_opa(m_container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

	m_weightLabel = lv_label_create(lv_scr_act(), nullptr);
	lv_label_set_text_fmt(m_weightLabel, "%0.02fg", 0.0f);
	lv_obj_set_style_local_text_font(m_weightLabel, 0, 0, &lv_font_montserrat_30);
	lv_obj_align(m_weightLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

	m_tareButton = lv_btn_create(m_container, nullptr);
	lv_obj_set_event_cb(m_tareButton, tare_btn_cb);
	lv_obj_set_user_data(m_tareButton, m_scalesEventLoop);
	lv_obj_align(m_tareButton, nullptr, LV_ALIGN_CENTER, 0, 70);

	lv_obj_t* tareBtnLabel = lv_label_create(m_tareButton, nullptr);
	lv_label_set_text(tareBtnLabel, "Tare");

	m_updateTask = lv_task_create(weightUpdateTask, 100, LV_TASK_PRIO_HIGHEST, (void*)this);
}

void ScalesScreen::updateWeight()
{
	auto weight = m_scalesEventLoop->getWeight();

	if (weight < 0)
		lv_label_set_text(m_weightLabel, "----");
	else
		lv_label_set_text_fmt(m_weightLabel, "%0.02fg", weight);

	lv_obj_align(m_weightLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
}
