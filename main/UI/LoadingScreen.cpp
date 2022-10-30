#include "LoadingScreen.hpp"

static void anim_text_opa_cb(void* var, int16_t v)
{
	auto label = static_cast<lv_obj_t*>(var);

	lv_obj_set_style_local_text_opa(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, v);
}

void LoadingScreen::init()
{
	m_spinner = lv_spinner_create(lv_scr_act(), nullptr);
	lv_obj_set_size(m_spinner, 35, 35);
	lv_obj_align(m_spinner, nullptr, LV_ALIGN_CENTER, 0, 65);
	lv_obj_set_style_local_line_width(m_spinner,LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, 4);
	lv_obj_set_style_local_line_width(m_spinner,LV_SPINNER_PART_BG, LV_STATE_DEFAULT, 4);

	m_title = lv_label_create(lv_scr_act(), nullptr);
	lv_label_set_text(m_title, "ESPresso Scales");
	lv_obj_set_style_local_text_font(m_title, 0, 0, &lv_font_montserrat_26);
	lv_obj_align(m_title, nullptr, LV_ALIGN_CENTER, 0, 0);

	m_label = lv_label_create(lv_scr_act(), nullptr);
	lv_label_set_text(m_label, "Loading...");
	lv_obj_align(m_label, nullptr, LV_ALIGN_CENTER, 0, 100);

	lv_anim_init(&m_loadingAnimation);
	lv_anim_set_var(&m_loadingAnimation, m_label);
	lv_anim_set_values(&m_loadingAnimation, 0xFF, 0x20);
	lv_anim_set_time(&m_loadingAnimation, 3000);
	lv_anim_set_playback_delay(&m_loadingAnimation, 100);
	lv_anim_set_playback_time(&m_loadingAnimation, 1000);
	lv_anim_set_repeat_delay(&m_loadingAnimation, 500);
	lv_anim_set_repeat_count(&m_loadingAnimation, LV_ANIM_REPEAT_INFINITE);

	lv_anim_set_exec_cb(&m_loadingAnimation, anim_text_opa_cb);
	lv_anim_start(&m_loadingAnimation);
}

void LoadingScreen::deactivate()
{
	lv_anim_del(&m_loadingAnimation, anim_text_opa_cb);

	lv_obj_del(m_label);
	lv_obj_del(m_spinner);

	m_spinner = nullptr;
	m_label = nullptr;
}
