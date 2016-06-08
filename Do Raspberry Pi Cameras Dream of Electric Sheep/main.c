//Aaron Ewing
//MSP430FR6989 with Rasberry Pi Camera V1.3 (OV5647 imaging sensor)
//Goal: Get Slave ID from Rasberry Pi Camera
///////////////////////////////////////////////////////////////////
// 					MSP430FR6989 		OV5647
// 					  master 			 slave
//
//					P1.6/SCL <--------> SCL0
//					P1.7/SDA <--------> SDA0
//						3.3V <--------> 3.3V
//						 GND <--------> GND
///////////////////////////////////////////////////////////////////
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
//based upon http://www.robot-electronics.co.uk/i2c-tutorial line 16 through 87

#define SCL
P1DIR |= 0x06 // I2C bus SCL = P1.6 outputs
#define SDA
P1DIR |= 0x07 // SDA = P1.7 outputs

//#define SCL_IN     P1DIR |= 0x06 // I2C bus SCL = P1.6
//#define SDA_IN     P1DIR |= 0x07 // SDA = P1.7

void i2c_dly(void)
{					// A slight delay to give a clear sequence between SDA and SCL changes.
}

void i2c_start(void) {
  SDA = 1;			// I2C start sequence
  i2c_dly();
  SCL = 1;
  i2c_dly();
  SDA = 0;
  i2c_dly();
  SCL = 0;
  i2c_dly();
}

void i2c_stop(void)
{
  SDA = 0;				// I2C stop sequence
  i2c_dly();
  SCL = 1;
  i2c_dly();
  SDA = 1;
  i2c_dly();
}

unsigned char i2c_rx(char ack)
{
char x, d=0;
  SDA = 1;
  for(x=0; x<8; x++) {
    d <<= 1;
    do {
      SCL = 1;
    }
    while(SCL_IN==0);		// wait for any SCL clock stretching
    i2c_dly();
    if(SDA_IN) d |= 1;
    SCL = 0;
  }
  if(ack) SDA = 0;
  else SDA = 1;
  SCL = 1;
  i2c_dly();			// send (N)ACK bit
  SCL = 0;
  SDA = 1;
  return d;
}

bool i2c_tx(unsigned char d)
{
char x;
static bool b;
  for(x=8; x; x--) {
    if(d&0x80) SDA = 1;
    else SDA = 0;
    SCL = 1;
    d <<= 1;
    SCL = 0;
  }
  SDA = 1;
  SCL = 1;
  i2c_dly();
  b = SDA_IN;			// possible ACK bit
  SCL = 0;
  return b;
}

// main
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    PM5CTL0 &=~ LOCKLPM5; // Disables high-impedance mode for FRAM memory

    int slaveID[100];		// poorly constructed array

    // starts segment to read in slave ID
    i2c_start();		// start
    i2c_tx(0x6C);		// AD+W
    // ack
    i2c_tx(0x3100);		// register to read from
    //ack
    i2c_start();		// restarts, preps for reading
    i2c_tx(0x6D);		// read register command
    //ack & data
    slaveID = i2c_rx(1);	// places slaveID into poorly constructed array
    i2c_stop();

	return 0;
}
