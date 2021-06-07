/*
  LED_Tile328.h - Arduino library for LED Tile, drawable 8x8 Dot Matrix LED
  based on Dots.h
  Copyright 2014 akita11 (akita@ifdl.jp). All right reserved

  Dots.h - Arduino library for 8x8/5x7 Dot LED Matrix.
  Copyright 2010 arms22. All right reserved.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*/

#ifndef LT328V2_H
#define LT328V2_H

// uncommenct "USE_ACC_MMA7660" for LT328v3 (using ADXL345)
//#define USE_ACC_MMA7660 // use Freescale's MMA7760 as accelerometer

#include <inttypes.h>

// Dot matrix LED: OSL641501-BRA] (Akizuki's I-05163)
// Connections:
// * ATmega328P = ArduinoNano/ArduinoMini/LT328 etc.
//  - Col1-8(Cathode) : PB0-1/PD2-7 (via 270ohm)
//  - Row1-8(Anode)   : PC0-5/PB3-4 (ADC0-7)

//   (0,0) ... (7,0) : pat[0](LSB=(0,0), MSB=(7,0)
//   ...       ...     ...
//   (0,7) ... (7,7) : pat[7](LSB=(0,0), MSB=(7,7)

#define LT_DRAW_MODE_NONE    0
#define LT_DRAW_MODE_DRAW    1
#define LT_DRAW_MODE_TOGGLE  2

#define PORT_ROW 0
#define PORT_COL 1
#define PORT_OUT 0
#define PORT_IN  1

#define COMU 0
#define COMD 1
#define COML 2
#define COMR 3
#define COM_TX 0x01
#define COM_RX 0x02
#define COM_RXBUF 16


#ifdef USE_ACC_MMA7660
#define ACC_ADDR 0x4c
#else
//#define ACC_ADDR 0x1d
#define ACC_ADDR 0x53
#endif

class LT
{
private:
  uint8_t _row, _col, _phase;
  uint8_t _th[8], _pat_in0[8];
  volatile uint8_t _raw_x, _raw_y, _raw_value, _raw_value_max, _f_raw_ready;
  void set_port_dir(int pos, int num, int dir);
  void set_port_value(int pos, int num, int value);

  void com_set_dir(uint8_t dir, uint8_t md);
  void com_set_pin(uint8_t dir, uint8_t md);
  uint8_t com_read_pin(uint8_t dir);
  uint8_t com_operation;
  uint8_t com_rbuf[4][COM_RXBUF];
  uint8_t com_pr[4];
  uint8_t com_pb[4];
  volatile uint8_t com_tst[4]; // tx status (0=idle, 1=8 in transmit)
  volatile uint8_t com_rst[4]; // rx status (0=idle, 1=8 in receive)
  uint8_t com_txd[4];
  void i2c_delay();
  void i2c_setup();
  void i2c_sendT();
  void i2c_sendP();
  uint8_t i2c_write(uint8_t d);
  uint8_t i2c_read(uint8_t ack);
  void setSDA(uint8_t x);
  void setSCL(uint8_t x);
  uint8_t readSDA();

public:
  LT();
  void update(void);

  uint8_t pat[8]; // output pattern
  uint8_t pat_in[8]; // input pattern
  uint8_t mode; // draw mode (LT_DRAW_MODE_{NONE,DRAW,TOGGLE}

  void begin(uint8_t f); // start LED Tile
  void end(void); // stop LED Tile

  void write(int x, int y, int value); // draw(1)/erase(0) dot at (x, y)
  void write(int y, int value); // set pat[y] as value
  void write(int y, const int values[], int size);  // fill pattern

  // functions for serial communication
  uint8_t read(int x, int y); // read 1/0 at (x, y)
  uint8_t read_raw(int x, int y); // read raw value at (x, y)
  uint8_t read_raw_max(int x, int y);  // raw max at (x, y) (in 30 cycle)
  uint8_t read_raw_max(int x, int y, int N);  // raw max at (x, y) in N cycle
  uint8_t read_raw_max(int N);  // raw max in all area in N cycle
  uint8_t read_raw_max(void);  // raw max in all area (in 30 cycle)

  void set_threshold(int th); // set single threshold for all columns
  void set_threshold(int th0, int th1, int th2, int th3,
		     int th4, int th5, int th6, int th7); // set each threshold for each column
  void set_threshold(int th[]); // set each threshold for each column
  void clear(void); // clear pattern

  int get_acc(uint8_t axis);

  int read_sw();  

  static LT *active_object;

  // functions for serial communication
  void com_write(uint8_t dir, uint8_t d);
  int com_available(uint8_t dir);
  uint8_t com_read(uint8_t dir);
  void com_process(void);
  //  uint8_t overflow();
};

#endif	// LT328V2.h
