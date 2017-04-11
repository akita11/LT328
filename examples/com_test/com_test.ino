#include <LT328v2.h>

LT lt = LT();

void setup()
{
  Serial.begin(9600);
  lt.begin(1);
  Serial.println("Ready");
}

void com_write_byte(byte dir, byte d)
{
  for (byte b = 0; b < 8; b++)  
    if (d & (1 << b)) lt.com_write(dir, 1);
    else lt.com_write(dir, 0);
}

byte com_available_byte(byte dir)
{
  if (lt.com_available(dir) > 8) return(1);
  else return(0);
}

byte com_read_byte(byte dir)
{
  if (com_available_byte(dir) > 0){
    byte d = 0;
    for (byte b = 0; b < 8; b++)
      if (lt.com_read(dir)) d |= (1 << b);
    return(d);
  }
  else return(0);
}

byte f = 0;

void loop()
{
  for (byte dir = 0; dir < 4; dir++){
    lt.com_write(dir, f);
    while (lt.com_available(dir) > 0){
      Serial.print(dir);
      Serial.print(' ');
      Serial.println(lt.com_read(dir), HEX);
    }
  }
/*
  for (byte dir = 0; dir < 4; dir++){
    com_write_byte(dir, 0x50 + dir);
    while (com_available_byte(dir) > 0){
      Serial.print(dir);
      Serial.print(' ');
      Serial.println(com_read_byte(dir), HEX);
    }
  }
*/
  delay(500);
}

