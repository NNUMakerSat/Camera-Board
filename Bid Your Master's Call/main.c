//Aaron Ewing
//MSP430FR6989 with Rasberry Pi Camera V1.3 (OV5647 imaging sensor)
//Goal: Get Slave ID from Rasberry Pi Camera
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
#include <stdint.h>
#include <stdbool.h>

//init_i2c and singlebytewrite based off of Braden Grim's work

void init_i2c(void) {
UCB1CTL1 |= UCSWRST; 							// Enable SW reset
UCB1CTL0 = UCMM + UCMST + UCMODE_3 + UCSYNC; 	// I2C multi-Master, synchronous mode
UCB1CTL1 = UCSSEL_2 + UCSWRST; 					// Use SMCLK, keep SW reset
UCB1BR0 = 10; 									// fSCL = SMCLK/10 = ~100kHz
UCB1BR1 = 0;	 								// UCB1I2COA |= 0x0630;
												// UCB1I2CSA = slaveAddress;
												// Slave Address is acc = 0x53, gyro = 0x68
//P4SEL |= BIT1 + BIT2; 							// Set P4.1,2 as sda, scl

//P1SEL0.6 = P1SEL1.6 = 1;						// P1.6 set for Tertiary module function
//P1SEL0.7 = P1SEL1.7 = 1;						// P1.7 set for Tertiary module function

P1SEL0 |= BIT6 + BIT7; 							// init P1.6 and 7, perifial not I/O
P1SEL1 |= BIT6 + BIT7;

//UCB1CTL1 &amp;= ~UCSWRST; 					// set eUSCI_B to Clear SW reset, resume operation
UCB1CTL1 &= ~UCSWRST; 							// set eUSCI_B to Clear SW reset, resume operation

UCB1IE = UCTXIE | UCRXIE | UCNACKIE; 			// Enable TXint RXint and the not acknowledge interrupt
}

void singleBytewrite(int slaveAddress, int registerAddress, int setBits) {
UCB1I2CSA = slaveAddress;						// Where the Master store's the Slaves ID
UCB1CTL1 |= UCTR + UCTXSTT;						// set eUSCI_B to transmitter mode then create START condition

//while ((UCB1IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
UCB1TXBUF = registerAddress;					// register address in transmission buffer

//UCB1CTL1 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition
//while ((UCB1IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
UCB1TXBUF = setBits;							// setBits in transmission buffer ***write data***
UCB1CTL1 |= UCTXSTP;							// set eUSCI_B to STOP condition
}

/*****main*****/
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    PM5CTL0 &=~ LOCKLPM5; 		// Disables high-impedance mode for FRAM memory

    int slaveAddress = 0x3100;		// slave address for OV5647 according to datasheet
    int registerAddress = 0x503D;		// register address for test pattern
    int setBits = 0x01;				// enable test pattern

//    P1SEL0 |= BIT6 + BIT7; 		// init P1.6 and 7, perifial not I/O
//    P1SEL1 |= BIT6 + BIT7;

    init_i2c(); 				// starts I2C function

    singleBytewrite(slaveAddress, registerAddress, setBits);			// starts function to write 1 byte to slave register

	return 0;
}

/* notes
 * I2CSAx is slave address (bit 9-0) table 26-17
 * remove LED jumper? (because share I2C pin?)
 *
 *ucbxctl modify only when ucswrst =1 ucmodex
 *
 * UCBxI2CSA holds slave name
 * transmit buffer ucb1txbuf
 */

/* REGISTER LIBRARY
 *
 * ALL for MSP430FR6989 unless otherwise stated
 *
 *-----init_i2c function-----
 *
 * UCBxCTL1 -> eUSCI_B (Enhanced Univeral Serial Communication Interface B, I2C & SPI, START, RESTART, STOP, low-power, interrupts, 7/10 bit options)
 *      (only touch eUSCI_B when UCSWRST = 1, transmitter/receiver modes for both master/slave,
 * UCSWRST -> Software reset enable (0 - disable, 1 - enabled/held in reset state)
 * (UCAx----- won't work unless USCWRST = 1 )
 * UCMM -> Multi-master environment (0b single master, 1b multiple masters)
 * UCMST -> Master slave select (0b slave, 1b master)
 * UCMODEx -> choose between SPI and I2C (00 - 3 pin SPI, 01&10 - 4 pin SPI, 11 - I2C)
 * UCSYNC -> synch mode (UCMODEx needs sync on)
 * UCSSELx -> (master and UCSWRST=1 only) selects BRCLK source clk (00 - UCLKI, 01 - ACLK, 10 - SMCLK, 11 - SMCLK)
 * UCBxBR0 -> bit clk prescaler (MSP430F5529) (UCSRST=1)
 * UCBRx -> bit clk prescaler (MSP430FR6989) (UCSRST=1)
 * PxSEL0&1 -> select port (P1SEL0.5 = 0 & P1SEL1.5 = 1 then primary module is selected for P1.5,
 *      (00 - General purpose I/O, 01- Primary module function is selected, 10 - secondary module, 11 - tertiary module))
 * &amp; -> used to escape
 * UCBxIE -> Interrupt Enable Register
 *      UCTXIE or UCTXIEx -> enable transmit interrupt (0 - disable, 1 - enable)
 *      UCRXIE or UCRXIEx -> enable receive interrupt (0 - disable, 1 - enable)
 *      UCNACKIE -> Not-acknowledge interrupt enable (0 - disable, 1 - enable)
 *
 *-----singlebytewrite function-----
 *
 * UCBxI2CSA -> I2D slave address, holds slave ID
 * UCBxCTL1 -> eUSCI_B (Enhanced Univeral Serial Communication Interface B, I2C & SPI, START, RESTART, STOP, low-power, interrupts, 7/10 bit options)
 *      (only touch eUSCI_B when UCSWRST = 1, transmitter/receiver modes for both master/slave,
 * UCTR -> transmitter/receiver for UCBxCTLW0 (0 - receiver, 1 - transmitter)
 * UCTXSTT -> transmit START condition as master (under UCBxCTLW0, ignored in slave mode, 0 - do not generate START condition, 1 - generate START)
 * UCTXSTP -> transmit STOP condition as master (under UCBxCTLW0, ignored in slave mode, 0 - do not generate STOP condition, 1 - generate STOP)
 * UCBxIFG -> Interrupt Flag Register
 * UCTXIFGx -> Transmit interrupt flag x (eUSCI_B, set when UXBxTXBUF is empty and slave add in UCBxI2COAx is on bus,
 *      (0 - no interrupt pending, 1 - interrupt pending))
 * UCBxTXBUF -> transmit buffer register,
 *      UCTXBUFx -> holds transmit data waiting to be moved into transmit shift register and transmitted,
 *           writting data to the transmit data buffer clears UCTXBUFx flags
 *
 *
 *
 */
