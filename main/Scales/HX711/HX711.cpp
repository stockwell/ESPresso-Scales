#include "HX711.hpp"

#include "esp_log.h"
#include <rom/ets_sys.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define CLOCK_DELAY_US 20

HX711Driver::HX711Driver(gpio_num_t dout, gpio_num_t sclk)
	: m_gpioPinDOUT(dout)
	, m_gpioPinSCLK(sclk)
{
	gpio_config_t io_conf = {};
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1UL << m_gpioPinSCLK);
	gpio_config(&io_conf);

	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1UL << m_gpioPinDOUT);
	gpio_config(&io_conf);

	setGain(m_gain);
}

bool HX711Driver::isReady()
{
	return gpio_get_level(m_gpioPinDOUT) == 0;
}

void HX711Driver::setGain(HX711Driver::HX711_GAIN gain)
{
	m_gain = gain;
	gpio_set_level(m_gpioPinSCLK, 0);
	read();
}

uint8_t HX711Driver::readByte()
{
	uint8_t value = 0;

	for (auto i = 0; i < 8; ++i)
	{
		gpio_set_level(m_gpioPinSCLK, 1);
		ets_delay_us(CLOCK_DELAY_US);
		value |= gpio_get_level(m_gpioPinDOUT) << (7 - i);
		gpio_set_level(m_gpioPinSCLK, 0);
		ets_delay_us(CLOCK_DELAY_US);
	}
	return value;
}

uint64_t HX711Driver::read()
{
	gpio_set_level(m_gpioPinSCLK, 0);

	// wait for the chip to become ready
	while (! isReady())
		vTaskDelay(1);

	uint64_t value = 0;
	uint8_t* value8 = static_cast<uint8_t*>((void*)&value);

	//--- Enter critical section ----
	portDISABLE_INTERRUPTS();

	value8[2] = readByte();
	value8[1] = readByte();
	value8[0] = readByte();

	// set the channel and the gain factor for the next reading using the clock pin
	for (auto i = 0; i < m_gain; i++)
	{
		gpio_set_level(m_gpioPinSCLK, 1);
		ets_delay_us(CLOCK_DELAY_US);
		gpio_set_level(m_gpioPinSCLK, 0);
		ets_delay_us(CLOCK_DELAY_US);
	}

	portENABLE_INTERRUPTS();

	value ^= 0x800000;

	return value;
}


uint64_t HX711Driver::readAverage(uint8_t times)
{
	uint64_t sum = 0;

	for (auto i = 0; i < times; i++)
		sum += read();

	return sum / times;
}

uint64_t HX711Driver::getValue(uint8_t times)
{
	uint64_t avg = readAverage(times);
	if (avg > m_offset)
		return avg - m_offset;
	else
		return 0;
}

float HX711Driver::getUnits(uint8_t times)
{
	return getValue(times) / m_scale;
}

void HX711Driver::tare()
{
	uint64_t sum = readAverage(20);
	setOffset(sum);
}

void HX711Driver::setScale(float scale)
{
	m_scale = scale;
}

float HX711Driver::getScale()
{
	return m_scale;
}

void HX711Driver::setOffset(uint64_t offset)
{
	m_offset = offset;
}

uint64_t HX711Driver::getOffset()
{
	return m_offset;
}

void HX711Driver::powerDown()
{
	gpio_set_level(m_gpioPinSCLK, 0);
	ets_delay_us(CLOCK_DELAY_US);
	gpio_set_level(m_gpioPinSCLK, 1);
	ets_delay_us(CLOCK_DELAY_US);
}

void HX711Driver::powerUp()
{
	gpio_set_level(m_gpioPinSCLK, 0);
}
