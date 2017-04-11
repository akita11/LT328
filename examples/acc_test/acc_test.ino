#include <LT328v2.h>

LT lt = LT();

void setup()
{
  Serial.begin(9600);
  lt.begin(0);
}

void loop()
{
  byte d;
  byte ax, ay, az;

  ax = lt.get_acc(0); // x-axis
  ay = lt.get_acc(1); // y-axis
  az = lt.get_acc(2); // z-axis

  Serial.print(" "); Serial.print(ax);
  Serial.print(" "); Serial.print(ay);
  Serial.print(" "); Serial.print(az);
  Serial.println("");
}

