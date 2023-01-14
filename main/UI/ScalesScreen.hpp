#pragma once

#include "ScalesEventLoop.hpp"

#include "lvgl.h"

class ScalesScreen
{
public:
	explicit ScalesScreen(ScalesEventLoop* scales);

	void activate();

	void updateWeight();
	void updateBattery();

	void shutdown();

private:
	ScalesEventLoop*	m_scalesEventLoop;

	lv_obj_t*			m_container;
	lv_obj_t*			m_weightLabel;
	lv_obj_t*			m_voltageLabel;
	lv_obj_t*			m_tareButton;
	lv_obj_t*			m_shutdownButton;

	lv_task_t*			m_weightTask = nullptr;
	lv_task_t*			m_batteryTask = nullptr;
};
