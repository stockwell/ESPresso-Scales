#pragma once

#include "lvgl.h"

#include <string>

class LoadingScreen
{
public:
	explicit LoadingScreen() = default;
	~LoadingScreen() = default;

	void init();

	void deactivate();

	void setLoadingText(const std::string& text);

private:
	lv_anim_t m_loadingAnimation;
	lv_obj_t* m_spinner = nullptr;
	lv_obj_t* m_label = nullptr;
	lv_obj_t* m_title = nullptr;
};
