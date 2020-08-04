#define TX_PIN 13
#define SIZE_OF_DATA 23
#define CHANGE_BT_PIN 2
#define LED_PIN 7

const unsigned long UP1_DATA[] =   {860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 900, 860};
const unsigned long UP2_DATA[] =   {860, 900, 860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 900, 860};
const unsigned long DOWN1_DATA[] = {860, 900, 860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 1750, 860};
const unsigned long DOWN2_DATA[] = {860, 900, 1750, 860, 900, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 1750, 900, 860, 900, 860, 1750, 860};
const unsigned long MUTE_ON_DATA[] = {950, 800, 950, 838, 1800, 800, 950, 800, 950, 800, 950, 800, 950, 800, 950, 800, 950, 1700, 900, 850, 1800, 1700, 950};
const unsigned long MUTE_OFF_DATA[] = {860, 900, 1750, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 900, 860, 1750, 860, 900, 1750, 1750, 860};

volatile byte _currentSignal = 0;
const unsigned long* SIGNALS[] = {UP1_DATA, UP2_DATA, DOWN1_DATA, DOWN2_DATA, MUTE_ON_DATA, MUTE_OFF_DATA};
volatile unsigned long* DATA = SIGNALS[0];
const byte SIZE_OF_SIGNALS = 6;
volatile bool _isClicked = false;
volatile unsigned long _lastClickedTime = 0;

void setup()
{
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, HIGH);
  pinMode(CHANGE_BT_PIN, INPUT_PULLUP);
  digitalWrite(CHANGE_BT_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(CHANGE_BT_PIN), onClick, FALLING);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void onClick()
{
  unsigned long curTime = millis();
  if (curTime - _lastClickedTime < 1000) return;
  _lastClickedTime = curTime;
  _currentSignal++;
  if (_currentSignal == SIZE_OF_SIGNALS) _currentSignal = 0;
  DATA = SIGNALS[_currentSignal];
  _isClicked = true;
}

void loop()
{
  for (int i = 0; i < SIZE_OF_DATA; i++)
  {
    if (_isClicked)
    {
      _isClicked = false;
      for(int j = 0; j <= _currentSignal; j++)
      {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
      delay(900);
      break;
    }
    digitalWrite(TX_PIN, !!(i % 2));
    delayMicroseconds(DATA[i]);
  }
  digitalWrite(TX_PIN, HIGH);
  delay(100);
}
