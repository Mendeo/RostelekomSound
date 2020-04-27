/*
 * RostelecomSound.cpp
 *
 * Created: 25.04.2020 16:49:27
 * Author : Themen
 */ 

#define F_CPU 9600000UL
#define RX_PIN INT0
#define ERROR_VALUE 13333UL
#define SIZE_OF_DATA 23
#define LED_PIN PORTB0
#define SELECTOR_PIN PORTB2 //Громкость вверх или вниз
#define CONTROL_PIN PORTB3  //Управление громкостью
#define SHORT_TIME 24000UL
#define LONG_TIME 45000UL

#define UP1_DATA      0b00011001
#define UP2_DATA      0b00011100
#define DOWN1_DATA    0b10011100
#define DOWN2_DATA    0b10011001
#define MUTE_ON_DATA  0b11100100
#define MUTE_OFF_DATA 0b11100001

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned long _RXPreviousTime = 0;
volatile unsigned long _pulseDuration = 0;
volatile uint8_t _up1Counter = 0;
volatile uint8_t _up2Counter = 0;
volatile uint8_t _down1Counter = 0;
volatile uint8_t _down2Counter = 0;
volatile uint8_t _muteOnCounter = 0;
volatile uint8_t _muteOffCounter = 0;
volatile bool _upFire = false;
volatile bool _downFire = false;
volatile bool _muteOnFire = false;
volatile bool _muteOffFire = false;

uint8_t _volumeLevel = 0;

volatile unsigned long _timer = 0; //1 us = _timer * (256 * 1E6 / F_CPU). Для нашего случая частота контроллера 9,6 МГц, Значит один инкремент переменной _timer случается примерно раз в 26,67 микросекунды.

ISR(TIM0_OVF_vect)
{
	_timer++;
}

inline unsigned long getExpectedTime(uint8_t data, uint8_t counter) //В зависимости от значения счётчика и паттерна (data) определяем какой длительности должен быть сигнал.
{
	uint8_t index;
	if (counter >= 2 && counter <= 4)
	{
		index = counter - 2;
	}
	else if (counter >= 15 && counter <= 17)
	{
		index = counter - 15;
	}
	else if (counter >= 20 && counter <= 21)
	{
		index = counter - 20;
	}
	else
	{
		return SHORT_TIME;
	}
	if (data & (1 << index)) return LONG_TIME;
	return SHORT_TIME;
}

bool incrementCounter(uint8_t data, volatile uint8_t *counter) //Увеличиваем счётчик, если приняли сигнал, который соответствует следующему значению в нашем паттерне (data). Если паттерн получен полностью, то возвращаем true.
{
	uint8_t PIN_STATUS = !!(PORTB & (1 << INT0)); //Аналог digitalRead на ардуино.
	if ((PIN_STATUS ^ (*counter % 2)) && _pulseDuration >= getExpectedTime(data, *counter) - ERROR_VALUE && _pulseDuration <= getExpectedTime(data, *counter) + ERROR_VALUE)
	{
		(*counter)++;
	}
	else
	{
		(*counter) = 0;
	}
	if (*counter == SIZE_OF_DATA)
	{
		(*counter) = 0;
		return true;
	}
	return false;
}

ISR(INT0_vect)
{
	_pulseDuration = _timer - _RXPreviousTime;
	_RXPreviousTime = _timer;
	if (incrementCounter(UP1_DATA, &_up1Counter))
	{
		_upFire = true;
	}
	else if (incrementCounter(UP2_DATA, &_up2Counter))
	{
		_upFire = true;
	}
	else if (incrementCounter(DOWN1_DATA, &_down1Counter))
	{
		_downFire = true;
	}
	else if (incrementCounter(DOWN2_DATA, &_down2Counter))
	{
		_downFire = true;
	}
	else if (incrementCounter(MUTE_ON_DATA, &_muteOnCounter))
	{
		_muteOnFire = true;
	}
	else if (incrementCounter(MUTE_OFF_DATA, &_muteOffCounter))
	{
		_muteOffFire = true;
	}
}

int main(void)
{
    DDRB = (1 << LED_PIN) | (1 << SELECTOR_PIN) | (1 << CONTROL_PIN); //Включаем LED_PIN, SELECTOR_PIN и CONTROL_PIN на выход, остальные на вход.
	GIMSK = (1 << INT0); //Настраиваем прерывание на INT0 (PB1).
	//Настраиваем прерывание по изменению уровня.
	MCUCR |= (1 << ISC00);
	MCUCR &= ~(1 << ISC01);	
	TIMSK0 = (1 << TOIE0); //Настраиваем прерывание по переполнению регистра таймера TCNT0.
	//Выставляем начальные значения пинов управления потенциометрами.
	PORTB &= ~(1 << SELECTOR_PIN) & ~(1 << CONTROL_PIN);
	sei(); //Разрешаем прерывания.
	bool isMute = false;
    while (true) 
    {
		if (_upFire && !isMute)
		{
			_upFire = false;			
			PORTB |= (1 << SELECTOR_PIN);
			PORTB |= (1 << CONTROL_PIN);
			_delay_ms(1);
			PORTB &= ~(1 << CONTROL_PIN);
			_delay_ms(1);
			_volumeLevel++;
		}
		else if (_downFire && !isMute)
		{
			_downFire = false;
			PORTB &= ~(1 << SELECTOR_PIN);
			PORTB |= (1 << CONTROL_PIN);
			_delay_ms(1);
			PORTB &= ~(1 << CONTROL_PIN);
			_delay_ms(1);
			_volumeLevel++;
		}
		else if (_muteOnFire && !isMute)
		{
			PORTB &= ~(1 << SELECTOR_PIN);
			for (uint8_t i = _volumeLevel; i > 0; i--)
			{
				PORTB |= (1 << CONTROL_PIN);
				_delay_ms(1);
				PORTB &= ~(1 << CONTROL_PIN);
				_delay_ms(1);
			}
		}
		else if (_muteOffFire && isMute)
		{
			PORTB |= (1 << SELECTOR_PIN);
			for (uint8_t i = 0; i < _volumeLevel; i++)
			{
				PORTB |= (1 << CONTROL_PIN);
				_delay_ms(1);
				PORTB &= ~(1 << CONTROL_PIN);
				_delay_ms(1);
			}
		}
    }
}