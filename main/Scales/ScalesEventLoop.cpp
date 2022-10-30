#include "ScalesEventLoop.hpp"

namespace
{
	enum Events : uint8_t
	{
		TickTimerElapsed,
		Init,
		Tare,
	};

	constexpr auto kPollPeriodMs = 20;

	constexpr gpio_num_t kGPIOPin_SCLK = GPIO_NUM_25;
	constexpr gpio_num_t kGPIOPin_DOUT1 = GPIO_NUM_26;
	constexpr gpio_num_t kGPIOPin_DOUT2 = GPIO_NUM_13;
}

ScalesEventLoop::ScalesEventLoop()
	: EventLoop("ScalesEvents")
{
	m_timer = std::make_unique<Timer>(kPollPeriodMs, [this]()
	{
		eventPost(Events::TickTimerElapsed);
	}, false);

	m_loadcells.emplace_back(kGPIOPin_DOUT1, kGPIOPin_SCLK);
}

void ScalesEventLoop::init()
{
	eventPost(Events::Init);
}

void ScalesEventLoop::tare()
{
	if (m_tareInProgress)
		return;

	m_tareInProgress = true;
	m_weight = -1;
	eventPost(Events::Tare);
}


void ScalesEventLoop::shutdown()
{

}

void ScalesEventLoop::eventHandler(int32_t eventId, void* data)
{
	switch(eventId)
	{
	case TickTimerElapsed:
		{
			float weight = 0.0f;
			for (auto& loadcell : m_loadcells)
				weight += loadcell.getUnits(5);

			m_weight = weight;

			m_timer->start();
			break;
		}

	case Init:
	{
		m_loadcells[0].setScale(1340.0f);

		for (auto& loadcell : m_loadcells)
			loadcell.tare();

		m_timer->start();

		m_ready = true;
		break;
	}

	case Tare:
		for (auto& loadcell : m_loadcells)
			loadcell.tare();

		m_tareInProgress = false;
		break;
	}

}
