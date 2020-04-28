#define DELAY_SIGNAL 1000
#define DATA_SIZE 5
unsigned long const data[] = {1000, 3000, 5000, 1000, 5000};

void setup()
{
  pinMode(7, INPUT);
  digitalWrite(7, HIGH);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop()
{
  if (!digitalRead(7))
  {
    for (int i = 0; i < DATA_SIZE; i++)
    {
      digitalWrite(13, i % 2);
      delay(data[i]);
    }
    digitalWrite(13, HIGH);
    delay(DELAY_SIGNAL);
  }
}
