/*
  LT328v2.cpp - Arduino library for LED Tile, drawable 8x8 Dot Matrix LED
  based on LED_Tile328.cpp, based onDots.cpp
  Copyright 2012 akita11 (akita@ifdl.jp). All right reserved

  Dots.cpp - Arduino library for 8x8/5x7 Dot LED Matrix.
  Copyright 2010 arms22. All right reserved.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*/

#include <pins_arduino.h>
#include <avr/interrupt.h>
#include <LT328v2.h>
//#include <Wire.h>

void LT::set_port_dir(int pos, int num, int dir)
{
//  for BRA
//  - Col1-8(Cathode) : PB0-1/PD2-7 (via 270ohm)
//  - Row1-8(Anode)   : PC0-5/PB3-4 (ADC0-7)
  if (pos == PORT_ROW){
    if (dir == PORT_OUT){
      if (num > 7){DDRC |= 0x3f; DDRB |= 0x18;}
      else{ if (num < 6) DDRC |= 1 << num; else DDRB |= 1 << (num - 3); }
    }
    else if (dir == PORT_IN){
      if (num > 7){DDRC &= ~0x3f; DDRB &= ~0x18;}
      else{ if (num < 6) DDRC &= ~(1 << num); else DDRB &= ~(1 << (num - 3));}
    }
  }
  else if (pos == PORT_COL){
    if (dir == PORT_OUT){
      if (num > 7){ DDRD |= 0xfc; DDRB |= 0x03;}
      else{ if (num < 2) DDRB |= 1 << num; else DDRD |= 1 << num; }
    }
    else{
      if (num > 7){ DDRD &= ~0xfc; DDRB &= ~0x03;}
      else{ if (num < 2) DDRB &= ~(1 << num); else DDRD &= ~(1 << num); }
    }
  }
}

void LT::set_port_value(int pos, int num, int value)
{
//  for BRA
//  - Col1-8(Cathode) : PB0-1/PD2-7 (via 270ohm)
//  - Row1-8(Anode)   : PC0-5/PB3-4 (ADC0-7)
  if (pos == PORT_ROW){
    if (value == 1){
      if (num > 7){PORTC |= 0x3f; PORTB |= 0x18;}
      else{ if (num < 6) PORTC |= 1 << num; else PORTB |= 1 << (num - 3); }
    }
    else{
      if (num > 7){PORTC &= ~0x3f; PORTB &= ~0x18;}
      else{
	if (num < 6) PORTC &= ~(1 << num); else PORTB &= ~(1 << (num - 3));
      }
    }
  }
  else if (pos == PORT_COL){
    if (value == 1){
      if (num > 7){ PORTD |= 0xfc; PORTB |= 0x03;}
      else{ if (num < 2) PORTB |= 1 << num; else PORTD |= 1 << num; }
    }
    else{
      if (num > 7){ PORTD &= ~0xfc; PORTB &= ~0x03;}
      else{ if (num < 2) PORTB &= ~(1 << num); else PORTD &= ~(1 << num); }
    }
  }
}

LT::LT(void)
{
  _row = 0; _col = 0;
  _phase = 0;
  _raw_x = 0;
  _raw_y = 0;
  _raw_value = 0;
  _raw_value_max = 0;
  _f_raw_ready = 1;
  for (int i = 0; i < 8; i++) _th[i] = 60; // set 'default' threshold
  mode = LT_DRAW_MODE_NONE;
}

void LT::begin(uint8_t f)
{
  uint8_t i;

  set_port_value(PORT_ROW, 8, 0);
  set_port_value(PORT_COL, 8, 0); //?
  set_port_dir(PORT_ROW, 8, PORT_OUT); 
  set_port_dir(PORT_COL, 8, PORT_OUT); 
  clear();

  if (f == 1){
    for (uint8_t dir = 0; dir < 4; dir++){
      com_set_dir(dir, COM_RX);
      com_tst[dir] = 0;
      com_rst[dir] = 0;
      com_pr[dir] = 0;
      com_pb[dir] = 0;
      //      com_prb[dir] = 0;
      //      com_pbb[dir] = 0;
    }
  }
  com_operation = f;

  //  pinMode(10, INPUT_PULLUP); // for SW
  DDRB |= 0x04; PORTB |= 0x04;
  
  LT::active_object = this;

  TCCR1A = 0; // clear control register A 
  TCCR1B = _BV(WGM13); // set mode as phase and frequency correct pwm, stop the timer
  //  ICR1 = 2804;
  //  ICR1 = 3300; // 8MHz/2/3300=1.2kHz:825us / 825us*3*8=19.8ms/frame
  ICR1 = 3500;
  //  ICR1 = 3840; // 23ms/frame (43.4fps)
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TCCR1B |= _BV(CS10); // no prescaler
  TIMSK1 = _BV(TOIE1); // sets the timer overflow interrupt enable bit

  DIDR0 = 0xff; // disable digital input for ADC0-7 input pins

  ADMUX = 0x60; // Vref=AVCC, left-adjust result
  //  ADMUX = 0xe0; // Vref=Vbg(1.1V), left-adjust result

  // ADC=13clk
  ADCSRA = 0x82; // ADC clock = 8MHz/4 = 2MHz
  ADCSRB |= 0x80; // ADC high speed mode
  ADCSRA |= _BV(ADSC); while ((ADCSRA & _BV(ADSC))); // initial conversion
  _f_raw_ready = 1;
  set_port_value(PORT_ROW, 8, 1); set_port_dir(PORT_ROW, 8, PORT_OUT);

  set_threshold(90); // default threshold
  i2c_setup();
#ifdef USE_ACC_MMA7660
  // initilization of MMA7660
  i2c_sendT();
  i2c_write(ACC_ADDR << 1);
  i2c_write(0x07); i2c_write(0x01); // active mode
  i2c_sendP();
#else
  // initialization of ADXL345
  i2c_sendT();
  i2c_write(ACC_ADDR << 1);
  //  i2c_write(0x16); i2c_write(0x25);
  i2c_write(0x2d); i2c_write(0x08); // Measure mode
  i2c_sendP();
  i2c_sendT();
  i2c_write(ACC_ADDR << 1);
  i2c_write(0x31); i2c_write(0x04); // +-2g, MSB-adjusted
  i2c_sendP();
#endif
}

void LT::end(void)
{
  TIMSK1 &= ~_BV(TOIE1); // disable Timer1 interrupt
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12)); // stop Timer1
  LT::active_object = 0;
}

void LT::write(int x, int y, int value)
{
  uint8_t msk;
  msk = 1 << x;
  if (value == 1) pat[y] |= msk; else pat[y] &= ~msk;
}

void LT::write(int y, int value)
{
  pat[y] = value;
}

void LT::write(int y, const int values[], int size)
{
  uint8_t i;
  for(i=0;i<size;i++)
    pat[(y++) & 0x07] = values[i];
}

uint8_t LT::read(int x, int y)
{
  if (pat_in[y] & (1 << x)) return(1); else return(0);
}

uint8_t LT::read_raw(int x, int y)
{
  _raw_x = x; _raw_y = y;
  _f_raw_ready = 0;
  while(_f_raw_ready == 0);
  return(_raw_value);
}

uint8_t LT::read_raw_max(int x, int y)
{
  return(read_raw_max(x, y, 30));
}

uint8_t LT::read_raw_max(int x, int y, int N)
{
  int i;
  uint8_t v;
  _raw_value_max = 0;
  for (i = 0; i < N; i++){
    v = read_raw(x, y);
    if (v > _raw_value_max) _raw_value_max = v;
  }
  return(_raw_value_max);
}

uint8_t LT::read_raw_max(int N)
{
  int i;
  uint8_t v;
  _raw_value_max = 0;
  _raw_x = 8;
  for (i = 0; i < N; i++){
    _f_raw_ready = 0;
    while(_f_raw_ready == 0);
  }
  return(_raw_value_max);
}

uint8_t LT::read_raw_max(void)
{
  return(read_raw_max(30));
}

void LT::clear(void)
{
  uint8_t y;
  for(y=0;y<8;y++){
    pat[y] = 0;
    pat_in[y] = 0;
    _pat_in0[y] = 0;
  }
  _row = 0; _phase = 0;
}

void LT::set_threshold(int th)
{
  for (int i = 0; i < 8; i++) _th[i] = th;
}

void LT::set_threshold(int th0, int th1, int th2, int th3,
		     int th4, int th5, int th6, int th7)
{
  _th[0] = th0; _th[1] = th1; _th[2] = th2; _th[3] = th3;
  _th[4] = th4; _th[5] = th5; _th[6] = th6; _th[7] = th7;
}

void LT::set_threshold(int th[])
{
  for (int i = 0; i < 8; i++) _th[i] = th[i];
}

void LT::update(void)
{
  int x;
  if (_phase == 0){
    _row = (_row + 1) % 8;
    _col = (_col + 1) % 8;
    // Reset (Row=0, Col=0)
    set_port_value(PORT_ROW, 8, 0); set_port_dir(PORT_ROW, 8, PORT_OUT);
    set_port_value(PORT_COL, 8, 0); set_port_dir(PORT_COL, 8, PORT_OUT);

    // Integrate (Col(selected)=0, Col(other)=Ain, Row=Ain)
    set_port_dir(PORT_COL, 8, PORT_IN);
    set_port_dir(PORT_ROW, 8, PORT_IN);
    set_port_value(PORT_ROW, 8, 0);
    set_port_value(PORT_COL, _col, 0);
    set_port_dir(PORT_COL, _col, PORT_OUT);
  }
  else if (_phase == 1){
    // scan
    uint8_t bm = 1 << _col;
    for (int y = 0; y < 8; y++){
      pat_in[y] &= ~bm;
      uint8_t a;
      uint8_t y0;
      ADMUX &= ~0x07; ADMUX |= (uint8_t)y;
      ADCSRA |= _BV(ADSC); // start conversion (dummy)
      while ((ADCSRA & _BV(ADSC))); // wait for finishing conversion (dummy)
      ADCSRA |= _BV(ADSC); // start conversion
      while ((ADCSRA & _BV(ADSC))); // wait for finishing conversion
      a = ADCH;
      if (a >= _th[y]){
	pat_in[y] |= bm;
	if (mode == LT_DRAW_MODE_DRAW) pat[y] |= bm;
	else if (mode == LT_DRAW_MODE_TOGGLE){
	  if (!(_pat_in0[y] & bm)){
	    pat[y] ^= bm;
	    _pat_in0[y] |= bm;
	  }
	}
      }
      else _pat_in0[y] &= bm;

      if (_f_raw_ready == 0){
	if (_col == _raw_x && y == _raw_y){
	  _raw_value = a;
	  _f_raw_ready = 1;
	}
        if (_raw_x == 8){
          if (a > _raw_value_max) _raw_value_max = a;
          if (_col == 7 && y == 7) _f_raw_ready = 1;
        }
      }
    }
    // display
    //    uint8_t mask = 1 << _row;
    set_port_value(PORT_ROW, 8, 0);
    set_port_dir(PORT_ROW, 8, PORT_OUT);
    set_port_value(PORT_COL, 8, 1);
    set_port_dir(PORT_COL, 8, PORT_OUT);

    for (uint8_t x = 0; x < 8; x++) if (pat[_row] & (1 << x)) set_port_value(PORT_COL, x, 0);
    set_port_value(PORT_ROW, _row, 1);
  }
  //  _phase = (_phase + 1) % 2;
  _phase = (_phase + 1) % 3;
}

LT *LT::active_object = 0;

ISR(TIMER1_OVF_vect)
{
  if(LT::active_object){
    LT::active_object->com_process();
    LT::active_object->update();
  }
}

#define N_BIT 8 // #cycles for one bit

void LT::com_process(void)
{
  if(LT::com_operation == 1){
    // COM transmit & receive function
    for (uint8_t dir = 0; dir < 4; dir++){
      if (com_tst[dir] > 0){
	// transmit process
	// tst        status
	// 0          idle
	// 1    - N   start bit
	// N+1  - 2N  bit#0(LSB)
	// 2N+1 - 3N  bit#1
	// ...
	// 8N+1 - 9N  bit#7(MSB)
	// 9N+1 - 10N stop bit
	if (com_tst[dir] > 0 && com_tst[dir] <= N_BIT){ // start bit
	  com_set_pin(dir, 0);
	  com_tst[dir]++;
	}
	else if (com_tst[dir] > N_BIT && com_tst[dir] <= 9*N_BIT){
	  if ((com_txd[dir] & 0x01) == 0) com_set_pin(dir, 0);
	  else com_set_pin(dir, 1);
	  if ((com_tst[dir] % N_BIT) == 0) com_txd[dir] = com_txd[dir] >> 1;
	  com_tst[dir]++;
	}
	else if (com_tst[dir] > 9*N_BIT && com_tst[dir] < 10*N_BIT){
	  com_set_pin(dir, 1);
	  com_tst[dir]++;
	}
	else{
	  com_tst[dir] = 0;
	  com_set_dir(dir, COM_RX);
	}
      }
      else{
	// receive process
	// rst        status
	// 0          idle
	// 1          1st '0' detected
	// 1+N/2      sampling start bit
	// 1+N/2+N    sampling bit#0(LSB)
	// 1+N/2+2N   sampling bit#1
	// ...
	// 1+N/2+8N   sampling bit#7(MSB)
	// 1+N/2+9N   sampling stop bit
	if (com_tst[dir] == 0 && com_rst[dir] == 0 && com_read_pin(dir) == 0){
	  com_rst[dir] = 1;
	  //	  com_rbuf[dir][com_pb[dir]] = 0;
	  com_rbuf[dir][com_pb[dir]] = 0x00;
	}
	else if (com_rst[dir] > 0){
	  if (com_rst[dir] == 1 + N_BIT/2){ // check start bit
	    if (com_read_pin(dir) == 1) com_rst[dir] = 0;
	    else com_rst[dir]++;
	  }
	  else if (com_rst[dir] > N_BIT && com_rst[dir] < 9*N_BIT && ((com_rst[dir] % N_BIT) == (N_BIT/2 + 1))){
	    com_rbuf[dir][com_pb[dir]] = com_rbuf[dir][com_pb[dir]] >> 1;
	    if (com_read_pin(dir) == 1) com_rbuf[dir][com_pb[dir]] |= 0x80;
	    else com_rbuf[dir][com_pb[dir]] &= ~0x80;
	    com_rst[dir]++;
	  }
	  else if (com_rst[dir] == 9*N_BIT + N_BIT/2 + 1){ // check stop bit
	    if (com_read_pin(dir) == 1){
	      com_rst[dir] = 0;
	      com_pb[dir]++;
	      if (com_pb[dir] == COM_RXBUF) com_pb[dir] = 0;
	    }
	  }
	  else{
	    com_rst[dir]++;
	  }
	}
      }
    }
  }
}

void LT::com_write(uint8_t dir, uint8_t d)
{
  while(com_tst[dir] != 0);
  while(com_rst[dir] != 0);
  com_txd[dir] = d;
  com_tst[dir] = 1;
  com_set_dir(dir, COM_TX);
}

int LT::com_available(uint8_t dir)
{
  if (com_pr[dir] == com_pb[dir]) return(0);
  else if (com_pr[dir] < com_pb[dir]) return(com_pb[dir] - com_pr[dir]);
  else return(com_pb[dir] + COM_RXBUF - com_pr[dir]);
}

uint8_t LT::com_read(uint8_t dir)
{
  uint8_t d;
  d = com_rbuf[dir][com_pr[dir]];
  com_pr[dir]++;
  if (com_pr[dir] == COM_RXBUF) com_pr[dir] = 0;
  return(d);
}

void LT::com_set_dir(uint8_t dir, uint8_t md)
{

  // COMU=PD0 COMD=PB5 COML=PD1 COMR=PB2
  if (md == COM_TX){
    switch (dir){
      case COMU: PORTD |= 0x01; DDRD |= 0x01; break;
      case COMD: PORTB |= 0x20; DDRB |= 0x20; break;
      case COML: PORTD |= 0x02; DDRD |= 0x02; break;
      case COMR: PORTB |= 0x04; DDRB |= 0x04; break;
    }
  }
  else if (md == COM_RX){
    switch (dir){
      case COMU: PORTD |= 0x01; DDRD &= ~0x01; break;
      case COMD: PORTB |= 0x20; DDRB &= ~0x20; break;
      case COML: PORTD |= 0x02; DDRD &= ~0x02; break;
      case COMR: PORTB |= 0x04; DDRB &= ~0x04; break;
    }
  }
}

void LT::com_set_pin(uint8_t dir, uint8_t md)
{
  if (md == 1){
    switch (dir){
      case COMU: PORTD |= 0x01; DDRD &= ~0x01; break;
      case COMD: PORTB |= 0x20; DDRB &= ~0x20; break;
      case COML: PORTD |= 0x02; DDRD &= ~0x02; break;
      case COMR: PORTB |= 0x04; DDRB &= ~0x04; break;
    }
  }
  else{
    switch (dir){
      case COMU: PORTD &= ~0x01; DDRD |= 0x01; break;
      case COMD: PORTB &= ~0x20; DDRB |= 0x20;  break;
      case COML: PORTD &= ~0x02; DDRD |= 0x02;  break;
      case COMR: PORTB &= ~0x04; DDRB |= 0x04;  break;
    }
  }
}

uint8_t LT::com_read_pin(uint8_t dir)
{
  switch (dir){
    case COMU: if (PIND & 0x01) return(1); else return(0); break;
    case COMD: if (PINB & 0x20) return(1); else return(0); break;
    case COML: if (PIND & 0x02) return(1); else return(0); break;
    case COMR: if (PINB & 0x04) return(1); else return(0); break;
  }
}

int LT::get_acc(uint8_t axis)
{
  int d = 0;
 
  if (axis >= 0 && axis <= 2){
    int f = 0;
    while(f == 0){
      i2c_sendT();
      i2c_write(ACC_ADDR << 1);
#ifdef USE_ACC_MMA7660
      i2c_write(0x00 + axis);
#else
      i2c_write(0x33 + (axis << 1));
#endif
      i2c_sendT();
      i2c_write(ACC_ADDR << 1 | 0x01);
      d = i2c_read(1);
      
#ifdef USE_ACC_MMA7660
      if ((d & 0x40) == 0) f = 1;
#else
      f = 1;
#endif
      i2c_sendP();
      f = 1;
    }
#ifdef USE_ACC_MMA7660
    d = (d << 2) & 0xfc;
#endif
  }
  if (d & 0x80) d = -((~d & 0xff) + 1);
#ifdef USE_ACC_MMA7660
#else
  d = -d; // invert sign for ADXL345
#endif
  return(d);
}

#define BM_SCL (1 << 6)
#define BM_SDA (1 << 7)

//#define SCL_IN()  {PORTB |= BM_SCL; DDRB &= ~BM_SCL;}
//#define SCL_OUT() {PORTB |= BM_SCL; DDRB |=  BM_SCL;}
//#define SDA_IN()  {PORTB |= BM_SDA; DDRB &= ~BM_SDA;}
////#define SDA_OUT() {PORTB |= BM_SDA; DDRB |=  BM_SDA;}
//#define SDA_OUT() {PORTB |= BM_SDA; DDRB &= ~BM_SDA;}
#define SCL_IN()  {}
#define SCL_OUT() {}
#define SDA_IN()  {}
#define SDA_OUT() {}

// SCL=PB6 SDA=PB7
void LT::setSDA(uint8_t x)
{
  if (x == 0){
    // drive 0 for '0'
    PORTB &= ~BM_SDA;
    DDRB |= BM_SDA;
  }
  else{
    // Z with pull-up for '1'
    PORTB |= BM_SDA;
    DDRB &= ~BM_SDA;
  }
}

void LT::setSCL(uint8_t x) {if (x == 0) PORTB &= ~BM_SCL; else PORTB |= BM_SCL;}
uint8_t LT::readSDA() {if (PINB & BM_SDA) return(1); else return(0);}

void LT::i2c_delay()
{
  //  delayMicroseconds(10);
  for (int i = 0; i < 10; i++) asm("nop");
}

void LT::i2c_setup()
{
  //  SCL_OUT(); SDA_IN();
  PORTB |= BM_SCL; DDRB |=  BM_SCL; // setup SCL pin
  PORTB |= BM_SDA; DDRB &= ~BM_SDA; // setup SDA pin
  SCL_OUT(); setSCL(1); setSDA(1);
}

void LT::i2c_sendT()
{
  SDA_OUT();
  setSCL(1); setSDA(1);
  i2c_delay();
  setSDA(0);
  i2c_delay();
  setSCL(0);
}

void LT::i2c_sendP()
{
  SDA_OUT();
  setSDA(0); setSCL(0);
  i2c_delay();
  setSCL(1);
  i2c_delay();
  setSDA(1);
  SDA_IN();
}

uint8_t LT::i2c_write(uint8_t d)
{
  int i;
  SDA_OUT();
  for (i = 0; i < 8; i++){
    if (d & 0x80) setSDA(1); else setSDA(0);
    d = d << 1;
    i2c_delay();
    setSCL(1);
    i2c_delay();
    setSCL(0);
  }
  SDA_IN();
  i2c_delay();
  setSCL(1);
  i2c_delay();
  if (readSDA() == 0) i = 0; else i = 1;
  setSCL(0);
  i2c_delay();
  SDA_IN();
  return(i);
}

uint8_t LT::i2c_read(uint8_t ack)
{
  int i;
  uint8_t d = 0;
  SDA_IN();
  for (i = 0; i < 8; i++){
    d = d << 1;
    if (readSDA() == 1) d |= 0x01;
    setSCL(1);
    i2c_delay();
    setSCL(0);
    i2c_delay();
  }
  SDA_OUT();
  //  setSDA(0); // ACK
  //  setSDA(1); // NAK
  setSDA(ack);
  i2c_delay();
  setSCL(1);
  i2c_delay();
  setSCL(0);
  i2c_delay();
  SDA_IN();
  return(d);
}

int LT::read_sw()
{
  if ((PINB & 0x04) == 0) return(1);
  else return(0);
}
