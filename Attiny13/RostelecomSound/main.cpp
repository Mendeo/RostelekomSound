/*
 * RostelecomSound.cpp
 *
 * Created: 25.04.2020 16:49:27
 * Author : Themen
 */ 

#define F_CPU 9600000UL //1200000UL //9600000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SIZE_OF_DATA 23
#define LED_PIN PORTB0
#define SELECTOR_PIN PORTB2 //Громкость вверх или вниз
#define CONTROL_PIN PORTB3  //Управление громкостью
#define RX_PIN PORTB1

#define ERROR_VALUE 19UL
#define SHORT_TIME 33UL
#define LONG_TIME 67UL
#define PAUSE_TIME 375UL //10000 мкс

#define LED_ON_TIME 938UL //25 мс

#define UP1_DATA      0b00011001
#define UP2_DATA      0b00011100
#define DOWN1_DATA    0b10011100
#define DOWN2_DATA    0b10011001
#define MUTE_ON_DATA  0b11100100
#define MUTE_OFF_DATA 0b11100001

#define HAS_PATTERN_START 0b00111111

#define SIZE_OF_PATTERNS 6

#define MAX_VOLUME 100 //10K по 0.1К => всего 100 возможных значений громкости.

volatile unsigned long _RXPreviousTime = 0;
volatile unsigned long _pulseDuration = 0;
volatile uint8_t _rxPinStatus = 0;
volatile bool _hasPulse = false;
uint8_t _counter = 0;
uint8_t _hasPattern = HAS_PATTERN_START; //Если время импулься совпадает с ожидаемым значением, то соответствующий бит остаётся равным единицы. Каждый бит соответствует одному из паттернов кнопок пульта.

const uint8_t PATTERNS[] = {UP1_DATA, UP2_DATA, DOWN1_DATA, DOWN2_DATA, MUTE_ON_DATA, MUTE_OFF_DATA};

volatile unsigned long _timer = 0;

ISR(TIM0_OVF_vect)
{
	_timer++;
}

ISR(INT0_vect)
{
	_pulseDuration = _timer - _RXPreviousTime;
	_RXPreviousTime = _timer;
	_hasPulse = true;
	_rxPinStatus = !!(PINB & (1 << RX_PIN));
}

unsigned long getExpectedTime(uint8_t data) //В зависимости от значения счётчика и паттерна (data) определяем какой длительности должен быть сигнал.
{
	uint8_t index;
	if (_counter >= 2 && _counter <= 4)
	{
		index = _counter - 2;
	}
	else if (_counter >= 15 && _counter <= 17)
	{
		index = _counter - 12;
	}
	else if (_counter >= 20 && _counter <= 21)
	{
		index = _counter - 14;
	}
	else
	{
		return SHORT_TIME;
	}
	if (data & (1 << index)) return LONG_TIME;
	return SHORT_TIME;
}

uint8_t incrementCounter() //Если паттерн получен полностью, то возвращаем номер кнопки в массиве PATTERNS.
{
	if (_pulseDuration > PAUSE_TIME)
	{
		_counter = 0;
		_hasPattern = HAS_PATTERN_START;
		return 255;
	}
	if (_hasPattern)
	{
		unsigned long eTime;
		for (uint8_t i = 0; i < SIZE_OF_PATTERNS; i++)
		{
			if (_hasPattern & (1 << i)) //Если раньше шаблон совпадал.
			{
				eTime = getExpectedTime(PATTERNS[i]);
				if (!((_rxPinStatus ^ !!(_counter % 2)) && _pulseDuration >= eTime - ERROR_VALUE && _pulseDuration <= eTime + ERROR_VALUE)) //Шаблон не совпадает.
				{
					_hasPattern &= ~(1 << i);
				}
			}
		}
		_counter++;
		if (_counter == SIZE_OF_DATA)
		{
			if (_hasPattern) //Какая-то кнопка совпала
			{
				
				switch (_hasPattern)
				{
					case 1: return 0;
					case 2: return 1;
					case 4: return 2;
					case 8: return 3;
					case 16: return 4;
					case 32: return 5;
					//Это про запас
					//case 64: return 6;
					//case 128: return 7;
					default: return 255;
				}
			}
			else
			{
				return 255;
			}
		}
		else
		{
			return 255; //Пока никакая кнопка не совпала
		}
	}
	else
	{
		return 255; //Никакая кнопка не совпала
	}
}

void doIncrement()
{
	PORTB &= ~(1 << CONTROL_PIN);
	_delay_ms(1);
	PORTB |= (1 << CONTROL_PIN);
	_delay_ms(1);
}

int main(void)
{
	DDRB = (1 << LED_PIN) | (1 << SELECTOR_PIN) | (1 << CONTROL_PIN); //Включаем LED_PIN, SELECTOR_PIN и CONTROL_PIN на выход, остальные на вход.
	GIMSK = (1 << INT0); //Настраиваем прерывание на INT0.
	//Настраиваем прерывание по изменению уровня.
	MCUCR |= (1 << ISC00);
	MCUCR &= ~(1 << ISC01);

	TIMSK0 = (1 << TOIE0); //Настраиваем прерывание по переполнению регистра таймера TCNT0.
	TCCR0B = (1 << CS00); //Настраиваем таймер на работу без делителя частоты.

	//Выставляем начальные значения пинов управления потенциометрами.
	PORTB &= ~(1 << SELECTOR_PIN);
	PORTB |= (1 << CONTROL_PIN);

	sei(); //Разрешаем прерывания.
	PORTB &= ~(1 << LED_PIN);
	bool isMute = false;
	uint8_t currentButton = 255;
	uint8_t volumeLevel = 0;
	unsigned long ledOnTime = 0;
	while (true)
	{
		//Выключаем светодиод, если он горит уже больше 25 мс
		if (ledOnTime > 0 && _timer - ledOnTime >= LED_ON_TIME)
		{
			PORTB &= ~(1 << LED_PIN);
			ledOnTime = 0;
		}
		if (_hasPulse)
		{
			_hasPulse = false;
			currentButton = incrementCounter();
			if (currentButton != 255)
			{
				//Включаем светодиод, если нажата кнопка на пульте.
				PORTB |= (1 << LED_PIN);
				ledOnTime = _timer;
				if ((currentButton == 0 || currentButton == 1) && volumeLevel < MAX_VOLUME) //Нажата кнопка вверх.
				{
					if (!isMute)
					{
						PORTB |= (1 << SELECTOR_PIN);
						doIncrement();
						volumeLevel++;
					}
				}
				else if ((currentButton == 2 || currentButton == 3) && volumeLevel > 0) //Нажата кнопка вниз.
				{
					if (!isMute)
					{
						PORTB &= ~(1 << SELECTOR_PIN);
						doIncrement();
						volumeLevel--;
					}
				}
				else if (currentButton == 4) //Включение mute.
				{
					if (!isMute)
					{
						isMute = true;
						PORTB &= ~(1 << SELECTOR_PIN);
						for (uint8_t i = volumeLevel; i > 0; i--)
						{
							doIncrement();
						}
					}
				}
				else if (currentButton == 5) //Выключение mute.
				{
					if (isMute)
					{
						isMute = false;
						PORTB |= (1 << SELECTOR_PIN);
						for (uint8_t i = 0; i < volumeLevel; i++)
						{
							doIncrement();
						}
					}
				}
			}
		}
	}
}