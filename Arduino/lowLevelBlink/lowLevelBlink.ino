#define LED_PIN PORTD7

volatile unsigned long _timer = 0; //Для нашего случая частота контроллера 9,6 МГц, Значит один инкремент переменной _timer случается примерно раз в 26,67 микросекунды.
volatile bool _isTime = false;
volatile unsigned long _prevTime = 0;

ISR(TIMER0_OVF_vect)
{
  _timer++;
  if (_timer - _prevTime >= 150000)
  {
    _prevTime = _timer;
    _isTime = true;
  }
}

int main(void)
{
  Serial.begin(9600);
  Serial.println(F_CPU);
  DDRD = (1 << LED_PIN); //Включаем LED_PIN, SELECTOR_PIN и CONTROL_PIN на выход, остальные на вход.
  TIMSK0 = (1 << TOIE0); //Настраиваем прерывание по переполнению регистра таймера TCNT0.
  TCCR0B = (1 << CS10); //Настраиваем таймер на работу без делителя частоты.
  sei(); //Разрешаем прерывания.
  PORTD |= (1 << LED_PIN);
  while (true)
  {
    if (_isTime)
    {
      _isTime = false;
      PORTD ^= (1 << LED_PIN);
    }
  }
}
