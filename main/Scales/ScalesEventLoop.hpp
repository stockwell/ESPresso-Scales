#pragma once

#include "Lib/EventLoop.hpp"
#include "Lib/Timer.hpp"

#include "HX711/HX711.hpp"

class ScalesEventLoop : public EventLoop
{
public:
	explicit ScalesEventLoop();
	virtual ~ScalesEventLoop() = default;

	void	init();
	void	tare();

	bool	ready() const		{ return m_ready; }
	float	getWeight() const	{ return m_weight; }

	void	shutdown();

protected:
	void						eventHandler(int32_t eventId, void* data) override;

private:
	std::unique_ptr<Timer>		m_timer;
	std::vector<HX711Driver>	m_loadcells;

	std::atomic<float>			m_weight = { 0.0f };
	std::atomic<bool>			m_ready = { false };

	bool 						m_tareInProgress = false;
};
