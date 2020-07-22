#define RX_PIN 2
#define ERROR_VALUE 500
#define SIZE_OF_DATA 23

volatile unsigned long _RXCurrentTime = 0;
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

const unsigned long UP1_DATA[] =   {860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 900, 860};
const unsigned long UP2_DATA[] =   {860, 900, 860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 900, 860};
const unsigned long DOWN1_DATA[] = {860, 900, 860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 1750, 860};
const unsigned long DOWN2_DATA[] = {860, 900, 1750, 860, 900, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 1750, 860};
const unsigned long MUTE_ON_DATA[] = {950, 800, 950, 838, 1800, 800, 950, 800, 950, 800, 950, 800, 950, 800, 950, 800, 950, 1700, 900, 850, 1800, 1700, 950};
const unsigned long MUTE_OFF_DATA[] = {860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 860, 900, 1750, 1750, 860};

void setup()
{
  Serial.begin(115200);
  Serial.println("Start listening");
  attachInterrupt(digitalPinToInterrupt(RX_PIN), onPulse, CHANGE);
  pinMode(RX_PIN, INPUT);
}

void onPulse()
{
  _RXCurrentTime = micros();
  _pulseDuration = _RXCurrentTime - _RXPreviousTime;
  _RXPreviousTime = _RXCurrentTime;
  if (incrementCounter(UP1_DATA, &_up1Counter) || incrementCounter(UP2_DATA, &_up2Counter)) _upFire = true;
  if (incrementCounter(DOWN1_DATA, &_down1Counter) || incrementCounter(DOWN2_DATA, &_down2Counter)) _downFire = true;
  if (incrementCounter(MUTE_ON_DATA, &_muteOnCounter)) _muteOnFire = true;
  if (incrementCounter(MUTE_OFF_DATA, &_muteOffCounter)) _muteOffFire = true;+
  Serial.println(_pulseDuration);
}

inline bool incrementCounter(const unsigned long data[], volatile uint8_t *counter)
{
  if ((digitalRead(RX_PIN) ^ (*counter % 2)) && _pulseDuration >= data[*counter] - ERROR_VALUE && _pulseDuration <= data[*counter] + ERROR_VALUE)
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

void loop()
{
  if (_upFire)
  {
    _upFire = false;
    detachInterrupt(digitalPinToInterrupt(RX_PIN));
    Serial.println("+");
    attachInterrupt(digitalPinToInterrupt(RX_PIN), onPulse, CHANGE);
  }
  if (_downFire)
  {
    _downFire = false;
    detachInterrupt(digitalPinToInterrupt(RX_PIN));
    Serial.println("-");
    attachInterrupt(digitalPinToInterrupt(RX_PIN), onPulse, CHANGE);
  }
  if (_muteOnFire)
  {
    _muteOnFire = false;
    detachInterrupt(digitalPinToInterrupt(RX_PIN));
    Serial.println("on");
    attachInterrupt(digitalPinToInterrupt(RX_PIN), onPulse, CHANGE);
  }
   if (_muteOffFire)
  {
    _muteOffFire = false;
    detachInterrupt(digitalPinToInterrupt(RX_PIN));
    Serial.println("off");
    attachInterrupt(digitalPinToInterrupt(RX_PIN), onPulse, CHANGE);
  }
}
