/*
 * RostelecomSound.cpp
 *
 * Created: 25.04.2020 16:49:27
 * Author : Themen
 */ 

#define LED_PIN PORTB3

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned long _timer = 0; //Для нашего случая частота контроллера 9,6 МГц, Значит один инкремент переменной _timer случается примерно раз в 26,67 микросекунды.
volatile bool _isTime = false;

ISR(TIM0_OVF_vect)
{
	_timer++;
	if (_timer % 150000 == 0)
	{
		_isTime = true;
	}
}

int main(void)
{
    DDRB = (1 << LED_PIN); //Включаем LED_PIN, SELECTOR_PIN и CONTROL_PIN на выход, остальные на вход.
	TIMSK0 = (1 << TOIE0); //Настраиваем прерывание по переполнению регистра таймера TCNT0.
	TCCR0B = (1 << CS00); //Настраиваем таймер на работу без делителя частоты.
	sei(); //Разрешаем прерывания.
	PORTB |= (1 << LED_PIN);
    while (true) 
    {
		if (_isTime)
		{
			_isTime = false;
			PORTB ^= (1 << LED_PIN);
		}
    }
}