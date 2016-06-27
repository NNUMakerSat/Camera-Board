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
#include "msp430fr6989.h" //not needed?
#include <stdint.h>
#include <stdbool.h>

uint16_t registerAddress;

void init_i2c(uint16_t slaveAddress);
//void init_i2c(uint8_t slaveAddress);

void singleBytewrite(uint16_t slaveAddress, uint16_t registerAddress, uint8_t setBits);
//void singleBytewrite(uint8_t slaveAddress, uint16_t registerAddress, uint8_t setBits);

//init_i2c, singlebytewrite, singleByteread based off of Braden Grim's work

/*****Initalizes I2C*****/
void init_i2c(uint16_t slaveAddress) {																				//no change in oscilliscope
//void init_i2c(uint8_t slaveAddress) {																				//no change in oscilliscope


	UCB0CTLW0 |= UCSWRST; 								// Enable SW reset (it is naturally, but double check)
	UCB0CTLW0 = UCSSEL_2 + UCSWRST; 					// Use SMCLK (undoes reset, so telling it to stay on reset)

	UCB0CTLW0 |= UCMST + UCMODE_3 + UCSYNC; 			// multi-Master, master mode, I2C, synchronous mode, use SMCLK(doesn't work?)
	//UCB0CTL0 |= UCMM + UCMST + UCMODE_3 + UCSYNC; 	// multi-Master, master mode, I2C, synchronous mode, use SMCLK(doesn't work?)
	//(is synch nesccary? looks like only for SPI)

	UCB0BRW = 10; 										// fSCL = SMCLK/10 = ~100kHz <messes with UCTXIFG0
	UCB0BR1 = 0;	 									// UCB1I2COA |= 0x0630;

	UCB0I2CSA = slaveAddress;							// Where the Master store's the Slaves ID (0x6C for write, 0x6D for read)				***changedx***
	UCB0CTLW0 &= ~UCSWRST; 								// set eUSCI_B to Clear SW reset, resume operation <start USCI state machine>

	//UCB0IE |= UCTXIE | UCRXIE | UCNACKIE; 				// Enable Transmit interrupt, receive interrupt, and the not acknowledge interrupt <look>
	UCB0IE |= UCTXIE0 | UCRXIE0 | UCNACKIE; 				// Enable Transmit interrupt, receive interrupt, and the not acknowledge interrupt <look>		/not sure which witch is which
	}

/*****Writes single byte in I2C*****/
/*
void singleBytewrite(uint16_t registerAddress, uint8_t setBits) {
	UCB0CTLW0 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition	 	&& should see something

	//while ((UCB0IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
	UCB0TXBUF = registerAddress;					// register address in transmission buffer							&& should see something

	//UCB0CTL1 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition
	//while ((UCB0IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
	UCB0TXBUF = setBits;							// setBits in transmission buffer ***write data***					&& should see something
	UCB0CTLW0 |= UCTXSTP;							// set eUSCI_B to STOP condition									&& should see something

//	UCB0CTLW1 |= UCTXSTP;							// set eUSCI_B to STOP condition									&& should see something
	}
	*/


void singleBytewrite(uint16_t slaveAddress, uint16_t registerAddress, uint8_t setBits) {
//void singleBytewrite(uint8_t slaveAddress, uint16_t registerAddress, uint8_t setBits) {
	UCB0CTLW0 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition	 	&& should see something

//	while (UCB0IE != 0 || UCB0TXBUF == 0) {		// while interrupt flag register and no interrupt pending
	UCB0TXBUF = slaveAddress;						// sends slave address?
//	}

	while (0){};						//empty loop to delay because delay cycles sucks when steping thru
//	_delay_cycles(1000);							// delays by 1ms for 1MHz
// wait and check the status of the buffer (is it empty yet?) <- current problem

//	while(UCB0TXIFG != 0);
//	while ((UCB0IE == 0) || (UCB0TXBUF != 0));		// while interrupt flag register and no interrupt pending
//	while ((UCB0IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
	//while (!(UCB0IE & UCTXIFG0));
	UCB0TXBUF = registerAddress;					// register address in transmission buffer							&& should see something

	//UCB0CTL1 |= UCTR + UCTXSTT;					// set eUSCI_B to transmitter mode then create START condition
	//while ((UCB0IFG &amp; UCTXIFG) == 0);			// while interrupt flag register and no interrupt pending
	while (!(UCB0IE & UCTXIFG0));
	UCB0TXBUF = setBits;							// setBits in transmission buffer ***write data***					&& should see something
	UCB0CTLW0 |= UCTXSTP;							// set eUSCI_B to STOP condition									&& should see something

//	UCB0CTLW1 |= UCTXSTP;							// set eUSCI_B to STOP condition									&& should see something
	}



/*****Reads single byte in I2C*****/ /*
uint8_t singleByteread(uint8_t registerAddress, uint16_t slaveAddress, uint8_t slaveAddress_read) {

	uint8_t data = 0;
	UCB0I2CSA = slaveAddress;					// 0x6C for reading address (should already be in there, but double check)
	UCB0CTLW0|= UCTR + UCTXSTT;						// set eUSCI_B to transmitter mode then create START condition	 	&& should see something
	UCB0CTLW0 &= ~UCTXSTT;							// stops sending start
	//no W0, but L1
	UCB0TXBUF = slaveAddress;					// sends slave address? <added>
	UCB0TXBUF = registerAddress;					// register address in transmission buffer							&& should see something

	UCB0CTLW0 |= UCTR + UCTXSTT;						// set eUSCI_B to transmitter mode then create START condition<added>	 	&& should see something
	UCB0CTLW0 &= ~UCTXSTT;							// stops sending start
	// no W0, but L1
//	UCB0I2CSA = slaveAddress_read;
	UCB0TXBUF = slaveAddress_read;					// sends slave read address? <added>
	UCB0CTLW0 &= ~UCTR;								// set eUSCI_B to receiver mode
	//no W0, but L1

	//while ((UCB0IFG &amp; UCRXIFG) == 0);
	data = UCB0RXBUF;
	UCB0IE |= UCNACKIE; 				// Enable the not acknowledge interrupt
	//UCB0CTL1 |= UCTXNACK;				// only for slave receiver mode
	UCB0CTLW0 |= UCTXSTP;
	UCB0CTLW0 &= ~UCTXSTP;							// stop sending STOP
	//no W0, but L1
	return data;
}
*/
/*****main*****/
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;						// Stop watchdog timer
   PM5CTL0 &= ~LOCKLPM5; 							// Disables high-impedance mode for FRAM memory
//   __bis_SR_register(P1SEL0 + P1SEL1 + GIE);

   uint16_t slaveAddress = 0x6C;						// slave address for writing to OV5647 according to datasheet (use 0x55 for testing)
 //  uint8_t slaveAddress = 0x6D;						// slave address for writing to OV5647 according to datasheet
   uint8_t slaveAddress_read = 0x6D;						// slave address for writing to OV5647 according to datasheet
   registerAddress = 0x3D;					// the last half of register address 0x503D
   uint8_t setBits = 0x01;								// enable test pattern

   P1SEL0 |= BIT6 + BIT7; 							// init P1.6 and 7, primary not I/O
   P1SEL1 &= ~BIT6 + ~BIT7;

   init_i2c(slaveAddress); 						// starts I2C function with OV5647 slave

   //singleBytewrite(registerAddress, setBits);		// starts function to write 1 byte to slave register
   singleBytewrite(slaveAddress, registerAddress, setBits);		// starts function to write 1 byte to slave register

 //  singleByteread(registerAddress, slaveAddress, slaveAddress_read);

   LPM0;											// Ends program in low power mode
}

/* things to keep in mind:
 * UCBIT9IFG
 * ISR for received data from slave
 *
 * UCA10 - 10 bit address mode
 * UCSLA10 - 10 bit slave mode
 * UCSAx - slave address bit
 *
 *
 *
 *
 */

/* notes
 * I2CSAx is slave address (bit 9-0) table 26-17
 * remove LED jumper? (because share I2C pin?)
 *
 *ucbxctl modify only when ucswrst =1 ucmodex
 *
 * UCBxI2CSA holds slave name
 * transmit buffer UCB0txbuf
 */



/* REGISTER LIBRARY
 *
 * ALL for MSP430FR6989 unless otherwise stated
 *
 *-----init_i2c function-----
 *
 * UCBxCTL1 -> eUSCI_B (Enhanced Univeral Serial Communication Interface B, I2C & SPI, START, RESTART, STOP, low-power, interrupts, 7/10 bit options)
 *      (only touch eUSCI_B when UCSWRST = 1, transmitter/receiver modes for both master/slave,
 * UCBxCTLW0&1 -> Control Word REgister (UCMM, UCMST, UCMODEx, UCSYNC, UCTXACK etc.)
 * UCSWRST -> Software reset enable (0 - disable, 1 - enabled/held in reset state)
 * (UCAx----- won't work unless USCWRST = 1 )
 * UCMM -> Multi-master environment (0b single master, 1b multiple masters)
 * UCMST -> Master slave select (0b slave, 1b master)
 * UCMODEx -> choose between SPI and I2C (00 - 3 pin SPI, 01&10 - 4 pin SPI, 11 - I2C)
 * UCSYNC -> synch mode (UCMODEx needs sync on)
 * UCSSELx -> (master and UCSWRST=1 only) selects BRCLK source clk (00 - UCLKI, 01 - ACLK, 10 - SMCLK, 11 - SMCLK)
 * UCBxBR0 -> bit clk prescaler (MSP430F5529) (UCSRST=1)
 * UCBRx -> bit clk prescaler (MSP430FR6989) (UCSRST=1)
 * PxSEL0&1 -> select port (P1SEL0 = 0 & P1SEL1 = 1 then primary module is selected,
 *      (00 - General purpose I/O, 01- Primary module function is selected, 10 - secondary module, 11 - tertiary module))
 * &amp; -> used to escape and not to be confused with &
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
