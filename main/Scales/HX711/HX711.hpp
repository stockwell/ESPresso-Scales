#pragma once

#include <hal/gpio_types.h>

class HX711Driver
{
public:
	enum HX711_GAIN
	{
		eGAIN_128 = 1,
		eGAIN_64 = 3,
		eGAIN_32 = 2
	};

	HX711Driver(gpio_num_t dout, gpio_num_t pd_sck);
	~HX711Driver() = default;

	bool isReady();
	void setGain(HX711_GAIN gain);

	uint64_t readAverage(uint8_t times);
	uint64_t getValue(uint8_t times);
	float getUnits(uint8_t times);
	void tare();

	void setScale(float scale);
	float getScale();

	void setOffset(uint64_t offset);
	uint64_t getOffset();

	void powerDown();
	void powerUp();

private:
	uint64_t read();
	uint8_t readByte();

private:
	gpio_num_t	m_gpioPinDOUT;
	gpio_num_t	m_gpioPinSCLK;
	HX711_GAIN	m_gain = HX711_GAIN::eGAIN_128;
	uint64_t	m_offset = 0;
	float		m_scale = 1.0f;
};




