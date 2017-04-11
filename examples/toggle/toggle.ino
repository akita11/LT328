#include <LT328v2.h>

LT lt = LT();

void setup()
{
  Serial.begin(9600);
  lt.begin(0);
  lt.set_threshold(35);
  lt.mode = LT_DRAW_MODE_TOGGLE; // set draw mode as 'toggle'
  Serial.println("Ready");
}

void loop()
{
  // read all columns' raw value
  Serial.print("Col: ");
  for (int x = 0; x < 8; x++){
    Serial.print(lt.read_raw(x, 0));
    Serial.print(" ");
  }
  Serial.println(' ');
//  Serial.print(": Max:");
  // read maximum raw value in whole area
//  Serial.println(lt.read_raw_max());
}


