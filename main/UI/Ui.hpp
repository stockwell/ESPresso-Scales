#pragma once

#include <string>
#include <memory>

#include "ScalesEventLoop.hpp"

#include "LoadingScreen.hpp"
#include "ScalesScreen.hpp"

#include "Lib/EventLoop.hpp"

class ESPressoScalesUI : public EventLoop
{
public:
	explicit ESPressoScalesUI(ScalesEventLoop* scales);

	void start();

protected:
	void							eventHandler(int32_t eventId, void* data) override;

private:
	void 							tick();

private:
	std::unique_ptr<Timer>			m_timer;

	LoadingScreen					m_loadingScreen;
	ScalesScreen					m_scalesScreen;

	ScalesEventLoop*				m_scales;
};
