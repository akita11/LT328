#include <LT328v2.h>

LT lt = LT();

void setup()
{
  Serial.begin(9600);
  lt.begin(0);
  Serial.println("Ready");
}

int f = 1;

void loop()
{
  byte d;
  byte ax, ay, az;

  int x, y;
  for (y = 0; y < 8; y++){
    for (x = 0; x < 8; x++){
      // read all columns' raw value
      Serial.print("Col: ");
        for (int x = 0; x < 8; x++){
        Serial.print(lt.read_raw(x, 0));
        Serial.print(" ");
      }
//      Serial.print(": Max:");
//      Serial.print(lt.read_raw_max());

      ax = lt.get_acc(0); // x-axis
      ay = lt.get_acc(1); // x-axis
      az = lt.get_acc(2); // x-axis
      Serial.print(" "); Serial.print(ax);
      Serial.print(" "); Serial.print(ay);
      Serial.print(" "); Serial.print(az);
      Serial.println("");
      lt.write(x, y, f);
      delay(100);
    }
  }
  f = 1 - f;
}

