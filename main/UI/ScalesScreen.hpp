#pragma once

#include "ScalesEventLoop.hpp"

#include "lvgl.h"

class ScalesScreen
{
public:
	explicit ScalesScreen(ScalesEventLoop* scales);

	void activate();

	void updateWeight();

private:
	ScalesEventLoop*	m_scalesEventLoop;

	lv_obj_t*			m_container;
	lv_obj_t*			m_weightLabel;
	lv_obj_t*			m_tareButton;

	lv_task_t*			m_updateTask = nullptr;
};
