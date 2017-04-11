#include <LT328v2.h>

LT lt = LT();

void setup()
{
  Serial.begin(9600);
  lt.begin(0);
  lt.write(0, 0, 1); // turn on (0, 0) 
  lt.write(2, 3, 1); // turn on (2, 3) 
  Serial.println("Ready");
}

void loop()
{
}


