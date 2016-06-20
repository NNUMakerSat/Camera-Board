//Aaron Ewing
//MSP430FR6989 with Rasberry Pi Camera V1.3 (OV5647 imaging sensor)
//Goal: Get Slave ID from Rasberry Pi Camera with I2C on Oscilliscope
///////////////////////////////////////////////////////////////////
// 					MSP430FR6989 		OV5647
// 					  master 			 slave
//
//					P1.7/SCL <--------> SCL0
//					P1.6/SDA <--------> SDA0
//						3.3V <--------> 3.3V
//						 GND <--------> GND
///////////////////////////////////////////////////////////////////
#include <msp430.h>
#include "msp430fr6989.h"
#include <stdint.h>
#include <stdbool.h>

uint16_t registerAddress;

void init_i2c(uint16_t slaveAddress);

void singleBytewrite(uint16_t slaveAddress, uint16_t registerAddress, uint8_t setBits);

//init_i2c, singlebytewrite, singleByteread based off of Braden Grim's work

/*****Initalizes I2C*****/
void init_i2c(uint16_t slaveAddress) {																							//&& no change in oscilliscope

	UCB0CTLW0 |= UCSWRST; 										// Enable SW reset (it is naturally, but double check)
	//UCB1CTL1 |= UCSWRST; 										// Enable SW reset (it is naturally, but double check)
	//UCB1CTLW0 = UCSSEL_2 + UCSWRST; 							// Use SMCLK (undoes reset, so telling it to stay on reset)
	//UCB1CTL1 = UCSSEL_2 + UCSWRST; 							// Use SMCLK (undoes reset, so telling it to stay on reset)
	UCB0CTLW0 |= UCMST + UCMODE_3 + UCSYNC; 				// multi-Master, master mode, I2C, synchronous mode, use SMCLK(doesn't work?)
	//UCB1CTL0 |= UCMM + UCMST + UCMODE_3 + UCSYNC; 				// multi-Master, master mode, I2C, synchronous mode, use SMCLK(doesn't work?)
	//(is synch nesccary? looks like only for SPI)

	UCB0BRW = 10; 										// fSCL = SMCLK/10 = ~100kHz
	UCB0BR1 = 0;	 									// UCB1I2COA |= 0x0630;
														// UCB1I2CSA = slaveAddress;4
														// Slave Address is acc = 0x53, gyro = 0x68

	UCB0I2CSA = slaveAddress;							// Where the Master store's the Slaves ID (0x6C for write, 0x6D for read)

	UCB0CTLW0 &= ~UCSWRST; 								// set eUSCI_B to Clear SW reset, resume operation <start USCI state machine>
	//UCB1CTL1 &= ~UCSWRST; 								// set eUSCI_B to Clear SW reset, resume operation <start USCI state machine>

	//UCB1IE |= UCTXIE | UCRXIE | UCNACKIE; 				// Enable Transmit interrupt, receive interrupt, and the not acknowledge interrupt <look>
	//UCB1IE |= UCTXIE0 | UCRXIE0 | UCNACKIE; 				// Enable Transmit interrupt, receive interrupt, and the not acknowledge interrupt <look>
	}


void singleBytewrite(uint16_t slaveAddress, uint16_t registerAddress, uint8_t setBits) {
	while (UCB0CTLW0 & UCTXSTP);     {          	// Ensure stop condition got sent
		UCB0CTLW0 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition	 	&& should see something
	}
	//UCB1CTLW0 &= ~UCTXSTT;							// stops sending start

	UCB1TXBUF = slaveAddress;					// sends slave address?
// wait  and check the status of the buffer (is it empty yet?)
	//while ((UCB1IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
	UCB1TXBUF = registerAddress;					// register address in transmission buffer							&& should see something

	//UCB1CTL1 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition
	//while ((UCB1IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
	UCB1TXBUF = setBits;							// setBits in transmission buffer ***write data***					&& should see something
	UCB1CTLW0 |= UCTXSTP;							// set eUSCI_B to STOP condition									&& should see something
	//UCB1CTLW0 &= ~UCTXSTP;							// stop sending STOP

//	UCB1CTLW1 |= UCTXSTP;							// set eUSCI_B to STOP condition									&& should see something
	}

/*****main*****/
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;						// Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; 							// Disables high-impedance mode for FRAM memory

    uint16_t slaveAddress = 0x3D;						// slave address for writing to OV5647 according to datasheet
    int registerAddress = 0x3D;							// register address for test pattern
    uint8_t setBits = 0x01;								// enable test pattern

    P1SEL0 |= BIT6 + BIT7; 							// init P1.6 and 7, primary not I/O
   // P1SEL1 |= BIT6 + BIT7;
   P1SEL1 &= ~BIT6 + ~BIT7;

    init_i2c(slaveAddress); 						// starts I2C function with OV5647 slave


    singleBytewrite(slaveAddress, registerAddress, setBits);		// starts function to write 1 byte to slave register

	return 0;											// Ends program in low power mode
}
