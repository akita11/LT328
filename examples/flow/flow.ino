#include <LT328v2.h>

LT lt = LT();

int ax, ay, az;
byte pu, pd, pl, pr;
int ax0, ay0, az0;

#define TH_ACC 25

byte bit_reverse(byte x)
{
  byte y = 0x00;
  for (int i = 0; i < 8; i++) if (x & (1 << i)) y |= 1 << (7-i);
  return(y);
}

void init_pattern()
{
  lt.write(3, 0x18); lt.write(4, 0x18);
  delay(100);
  lt.write(2, 0x3c); lt.write(3, 0x24); lt.write(4, 0x24); lt.write(5, 0x3c);
  delay(100);
  lt.write(1, 0x7e); for (int y = 2; y < 6; y++) lt.write(y, 0x42); lt.write(6, 0x7e);
  delay(100);
  lt.write(0, 0xff); for (int y = 1; y < 7; y++) lt.write(y, 0x81); lt.write(7, 0xff);
  delay(100);
  lt.clear();  
}

void setup()
{
  lt.begin(1);

  init_pattern();
  ax0 = lt.get_acc(0);
  ay0 = lt.get_acc(1);
  
  lt.mode = LT_DRAW_MODE_DRAW;

  //  Serial.begin(9600); Serial.println("Ready");
}

//           (0)...(7)
// (7) pat[0] 0 ... 7 (0) 
//     ...
// (0) pat[7] 0 ... 7 (7)
//           (7)...(0)

void loop()
{
  int i;
  ax = lt.get_acc(0) - ax0;
  ay = lt.get_acc(1) - ay0;
  //  Serial.print(ax); Serial.print(' '); Serial.print(ax0); Serial.print(' | '); Serial.print(ay); Serial.print(' '); Serial.println(ay0);
  if (ay > TH_ACC){
    // to left
    pl = 0x00; for (i = 0; i < 8; i++) if (lt.pat[i] & 0x01) pl |= 1 << (7-i);
    for (i = 0; i < 8; i++) lt.pat[i] = lt.pat[i] >> 1;
    for (i = 0; i < 8; i++) if (pr & 1 << i) lt.pat[i] |= 0x80;
    pr = 0;
    lt.com_write(COML, pl);
//    SerialR.print("L"); SerialR.println(pl, HEX); 
  }
  else if (ay < -TH_ACC){
    // to right
    pr = 0x00; for (i = 0; i < 8; i++) if (lt.pat[i] & 0x80) pr |= 1 << i;
    for (i = 0; i < 8; i++) lt.pat[i] = lt.pat[i] << 1;
    for (i = 0; i < 8; i++) if (pl & 1 << i) lt.pat[7-i] |= 0x01;
    pl = 0;
    lt.com_write(COMR, pr);   
//    SerialR.print("R"); SerialR.println(pr, HEX); 
  }
  if (ax < -TH_ACC){
    // to down
    pd = 0x00; for (i = 0; i < 8; i++) if (lt.pat[7] & 1 << i) pd |= 1 << (7-i);
    // 7<-6 6<-5 ... 1<-0
    for (i = 0; i < 7; i++) lt.pat[7-i] = lt.pat[6-i];
    lt.pat[0] = pu;
    pu = 0;
    lt.com_write(COMD, pd);
 //   SerialR.print("D"); SerialR.println(pd, HEX); 
  }
  else if (ax > TH_ACC){
    // to up
    pu = lt.pat[0];
    for (i = 0; i < 7; i++) lt.pat[i] = lt.pat[i+1];
    lt.pat[7] = 0; for (i = 0; i < 8; i++) if (pd & 1 << i) lt.pat[7] |= 1 << (7-i);
    pd = 0;
    lt.com_write(COMU, pu);
//    SerialR.print("U"); SerialR.println(pu, HEX); 
  }
  if (lt.com_available(COMU) > 0) pu = bit_reverse(lt.com_read(COMU));
  if (lt.com_available(COMD) > 0) pd = bit_reverse(lt.com_read(COMD));
  if (lt.com_available(COML) > 0) pl = bit_reverse(lt.com_read(COML));
  if (lt.com_available(COMR) > 0) pr = bit_reverse(lt.com_read(COMR));
  delay(100);

  if (read_sw() == 1){
    init_pattern();
    ax0 = lt.get_acc(0);
    ay0 = lt.get_acc(1);
    while(read_sw() == 1) delay(10);
  }
}
