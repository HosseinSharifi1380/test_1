/*
 * main.c
 *
 *  Created on: Sep 19, 2024
 *      Author: Asus
 */


#include "xparameters.h"	/* XPAR parameters */
#include "xspi.h"			/* SPI device driver */
#include "xspi_l.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define BUFFER_SIZE			10

/************************** Variable Definitions *****************************/
XSpi  SpiInstance;	 	/* The instance of the SPI device */
u8 	  WriteBuffer[BUFFER_SIZE];

int main(void)
{
	XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

	// Initialize the SPI driver so that it is  ready to use.
	ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
	XSpi_CfgInitialize(&SpiInstance, ConfigPtr, ConfigPtr->BaseAddress);

	// Set the Spi device as a master
	XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION| XSP_MANUAL_SSELECT_OPTION );

	// Start the SPI driver so that the device is enabled.
	XSpi_Start(&SpiInstance);

	//select slave number which wanted to transmit data//
	XSpi_SetSlaveSelect(&SpiInstance, 1);

	// Disable Global interrupt to use polled mode operation
	XSpi_IntrGlobalDisable(&SpiInstance);

	// Initialize the write buffer with pattern to write
	for (u8 i = 0; i < BUFFER_SIZE; i++)
		WriteBuffer[i] = i;

	// Transmit the data.
	XSpi_Transfer(&SpiInstance, WriteBuffer, NULL, BUFFER_SIZE);
	usleep(1);
	XSpi_Transfer(&SpiInstance, WriteBuffer, NULL, BUFFER_SIZE);


	/*********************** Low Level ***********************/
	u32 Control;
	XSpi_WriteReg(SpiInstance.BaseAddr, XSP_SSR_OFFSET, 0);

	for(int i=0; i<BUFFER_SIZE; i++)
	{
		XSpi_WriteReg(SpiInstance.BaseAddr, XSP_DTR_OFFSET, WriteBuffer[i]);


		/* Enable the device. */
		Control = XSpi_ReadReg(SpiInstance.BaseAddr, XSP_CR_OFFSET);
		Control |= XSP_CR_ENABLE_MASK;
		Control &= ~XSP_CR_TRANS_INHIBIT_MASK;
		XSpi_WriteReg(SpiInstance.BaseAddr, XSP_CR_OFFSET, Control);

		while (!(Xil_In32(SpiInstance.BaseAddr + XSP_SR_OFFSET) & XSP_SR_TX_EMPTY_MASK));
	}



	XSpi_WriteReg(SpiInstance.BaseAddr, XSP_SSR_OFFSET, 0xf);
}
