#include "nRF24L01+.h"
#include "spi.h"
#include "error.h"
#include "rti.h"

// SPI COMMANDS
#define R_REGISTER(x) (x)
#define W_REGISTER(x) (BIT(5) | x)

#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2

#define REUSE_TX_PL 0xE3
#define R_RX_PL_WID 0x60
#define W_ACK_PAYLOAD(x) (0xA8 | x)
#define W_TX_PAYLOAD_NO_ACK 0xB0

#define ACTIVATE 0x50
#define ACTIVATE_DATA 0x73

#define NOP 0xFF

// REGISTER ADDRESSES & BITS
#define CONFIG 0x00
#define MASK_RX_DR BIT(6)
#define MASK_TX_DS BIT(5)
#define MASK_MAX_RT BIT(4)
#define EN_CRC BIT(3)
#define CRC_1_BYTE (0 << 2)
#define CRC_2_BYTE (1 << 2)
#define PWR_UP BIT(1)
#define PRIM_RX BIT(0)
#define PRIM_TX (0 << 0)

#define EN_AA 0x01 // Enable Auto Acknowledgement
#define ENAA_P(x) BIT(x)

#define EN_RXADDR 0x02 // Enable RX Addresses
#define ERX_P(x) BIT(x)

#define SETUP_AW 0x03 // Setup of Address Width
#define AW_3_BYTES 0x01
#define AW_4_BYTES 0x02
#define AW_5_BYTES 0x03

#define SETUP_RETR 0x04// Setup of Automatic Retransmission
#define ARD(x) (x<<4) // The delay is equal to (x+1) * 250us
#define ARC(x) x // x equals the maximum amount of retransmissions

#define RF_CH 0x05// RF Channel
#define SET_CH(x) x // The channel frequency is 2400 + x MHz (1 MHz resolution)

#define RF_SETUP 0x06// RF Setup
#define CONT_WAVE BIT(7)
#define RF_DR_1MBPS ((0 << 5) | (0 << 3)) 
#define RF_DR_2MBPS  ((0 << 5) | (1 << 3)) 
#define RF_DR_250KBPS  ((1 << 5) | (0 << 3)) 
#define PLL_LOCK BIT(4)
#define RF_PWR_m18DBM ((0x00) << 1)
#define RF_PWR_m12DBM ((0x01) << 1)
#define RF_PWR_m6DBM ((0x02) << 1)
#define RF_PWR_0DBM ((0x03) << 1)

#define STATUS 0x07
#define RX_DR BIT(6) // Data Ready RX FIFO interrupt
#define TX_DS BIT(5) // Data Sent TX FIFO interrupt
#define MAX_RT BIT(4) // Maximum number of retransmits reached interrupt
#define RX_P_NO(x) (x << 1) // Data pipe number for the payload available for reading in the RX FIFO
#define ST_TX_FULL BIT(0) // TX FIFO full

#define OBSERVE_TX 0x08
#define PLOS_CNT 0xF0 // Lost packets
#define ARC_CNT 0x0F // Retransmitted packets. Reset for each new transmission

#define RPD 0x09
#define RPD_BIT BIT(0) // Received Power Detector (Carrier Detect)

#define RX_ADDR_P(x) (0x0A + x) // Pipe 0 receive address
// Pipe 0 has a length defined by SETUP_AW. All other pipes share the most significant bytes, 
// and differ in the least significant byte.
// This value must be set equal to the PTX's TX_ADDR if automatic acknowledgement is being used.

#define TX_ADDR 0x10 // Length defined by SETUP_AW

#define RX_PW_P(x) (0x11 + x) // RX payload width for pipe x

#define FIFO_STATUS 0x17
#define TX_REUSE BIT(6)
#define TX_FULL BIT(5) // TX FIFO Full
#define TX_EMPTY BIT(4) // TX FIFO Empty
#define RX_FULL BIT(1) // RX FIFO Full
#define RX_EMPTY BIT(0) // RX FIFO Empty

#define DYNPD 0x1C
#define DPL_P(x) BIT(x)  // Dynamic Payload Length for pipe x

#define FEATURE 0x1D
#define EN_DPL BIT(2) // Enable Dynamic Payload Length
#define EN_ACK_PAY BIT(1) // Enable Payload with ACK
#define EN_DYN_ACK BIT(0) // Enable the W_TX_PAYLOAD_NO_ACK command

#define NRF_SPI_CPOL 0
#define NRF_SPI_CPHA 0

#define NRF_SPI_DATA_SIZE 40

#define NRF_STARTUP_TIME_MS 150
#define NRF_CE_PULSE_DURATION_MS 2 // In reality, it is about 10us, but this is only done once so no harm done

#define NRF_ADDRESS 0xE7

#define ENABLE_IRQ() (IRQCR_IRQEN = 1)
#define DISABLE_IRQ() (IRQCR_IRQEN = 0)

typedef struct
{
	bool transmitting;
	u8 callbackStage;
	nrf_PTXptr eot;
	
	// Used when reading the payload in an ACK
	u8 readLen[2];
} nrf_PTXData;

typedef struct
{
	u8 callbackStage;
	nrf_PRXptr eot;
	bool receiveRequest;
	bool sendRequest;
	
	// Used for temporary payload storage
	u8 *data;
	u8 length;
	
	// Used when reading the payload
	u8 readLen[2];
} nrf_PRXData;

struct {
	nrf_Type type;
	u8 spiInputData[NRF_SPI_DATA_SIZE];
	u8 spiOutputData[NRF_SPI_DATA_SIZE];
	nrf_PTXData ptxData;
	nrf_PRXData prxData;
} nrf_data;

bool nrf_isInit = _FALSE;
u8 nrf_initStep;

void nrf_rtiInitCallback (void *data, rti_time period, rti_id id);
void nrf_InitSequence (void);
void nrf_CommenceTransmission (u8 *data, u8 length);
void nrf_spiReadStatusCallback (void);
void nrf_spiHandleTXCallback (void);
void nrf_spiHandleTXACKCallback (void);
void nrf_spiHandleMAXRTCallback (void);
void nrf_spiHandleRXCallback (void);
void nrf_StoreAckDone (void);

void nrf_Init (nrf_Type type)
{
	if (nrf_isInit == _TRUE)
		return;
		
	nrf_data.type = type;
	
	if (nrf_data.type == PTX)
		nrf_data.ptxData.transmitting = _FALSE;
	else // if PRX
	{
		nrf_data.prxData.eot = NULL;
		nrf_data.prxData.receiveRequest = _FALSE;
		nrf_data.prxData.sendRequest = _FALSE;
	}
	
	NRF_CE_DDR = DDR_OUT;
	NRF_CE = 0;
	
	rti_Init();
	nrf_initStep = 0;
	rti_Register(nrf_rtiInitCallback, NULL, RTI_ONCE, RTI_MS_TO_TICKS(NRF_STARTUP_TIME_MS));

	while (nrf_isInit != _TRUE)
		;
			
	return;
}

void nrf_rtiInitCallback (void *data, rti_time period, rti_id id)
{
	nrf_InitSequence();
}

void nrf_InitSequence (void)
{	
	switch (nrf_initStep)
	{
		// Device is in the Power Down state if there was a power cycle, or in some other state if the uC was reset.
		case 0:
			nrf_initStep ++;
			
			spi_Init(NRF_SPI_CPOL, NRF_SPI_CPHA);
			
			nrf_data.spiInputData[0] = W_REGISTER(CONFIG);
			// Enable all interrupts (all masks are set to 0), enable 2 byte CRC, power down device (PWR_UP = 0), and set as PTX/PRX.
			nrf_data.spiInputData[1] = EN_CRC | CRC_2_BYTE | ((nrf_data.type == PRX) ? PRIM_RX : PRIM_TX);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
			
			break;
			
		case 1:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(EN_AA);
			// Enable Auto Acknowledgement for Pipe 0 (and disable for all other pipes).
			nrf_data.spiInputData[1] = ENAA_P(0);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
			
			break;
			
		case 2:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(EN_RXADDR);
			// Enable RX Pipe 0 (and disable for all other pipes).
			nrf_data.spiInputData[1] = ERX_P(0);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
				
		case 3:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(SETUP_AW);
			// Set Address Width to 5 bytes
			nrf_data.spiInputData[1] = AW_5_BYTES;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 4:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(SETUP_RETR);
			// Set ARD to (1+1) * 500us = 250us, and ARC to 10 retransmits.
			nrf_data.spiInputData[1] = ARD(1) | ARC(10);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 5:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(RF_CH);
			// Set RF Channel to 24050 MHz
			nrf_data.spiInputData[1] = SET_CH(50);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 6:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(RF_SETUP);
			// Set Data Rate to 2Mbps and output power to 0dBm.
			nrf_data.spiInputData[1] = RF_DR_2MBPS | RF_PWR_0DBM;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 7:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(RX_ADDR_P(0));
			// Set RX Address
			nrf_data.spiInputData[1] = NRF_ADDRESS;
			nrf_data.spiInputData[2] = NRF_ADDRESS;
			nrf_data.spiInputData[3] = NRF_ADDRESS;
			nrf_data.spiInputData[4] = NRF_ADDRESS;
			nrf_data.spiInputData[5] = NRF_ADDRESS;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 6, nrf_InitSequence);
		
			break;
			
		case 8:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(TX_ADDR);
			// Set TX Address
			nrf_data.spiInputData[1] = RF_DR_2MBPS | RF_PWR_0DBM;
			nrf_data.spiInputData[1] = NRF_ADDRESS;
			nrf_data.spiInputData[2] = NRF_ADDRESS;
			nrf_data.spiInputData[3] = NRF_ADDRESS;
			nrf_data.spiInputData[4] = NRF_ADDRESS;
			nrf_data.spiInputData[5] = NRF_ADDRESS;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 6, nrf_InitSequence);
		
			break;
			
		case 9:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(DYNPD);
			// Enable dynamic payload length for Pipe 0.
			nrf_data.spiInputData[1] = DPL_P(0);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
		
		case 10:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = ACTIVATE;
			// Enable the R_RX_PL_WID, W_ACK_PAYLOAD and W_TX_PAYLOAD_NOACK commands
			nrf_data.spiInputData[1] = ACTIVATE_DATA;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 11:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(FEATURE);
			// Enable dynamic payload length, enable payload with ACK, disable transmissions with no ACK.
			nrf_data.spiInputData[1] = EN_DPL | EN_ACK_PAY;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 12:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(STATUS);
			// Turn off all interrupts, if they were on (because the nRF wasn't reset)
			nrf_data.spiInputData[1] = MAX_RT | TX_DS | RX_DR;
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
		
			break;
			
		case 13:
			nrf_initStep ++;
						
			nrf_data.spiInputData[0] = W_REGISTER(CONFIG);
			// Register configuration is done, power on device.
			nrf_data.spiInputData[1] = EN_CRC | CRC_2_BYTE | PWR_UP | ((nrf_data.type == PRX) ? PRIM_RX : PRIM_TX);
			
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_InitSequence);
			
			break;
		
		case 14:
			nrf_initStep ++;
			NRF_CE = 1; 
			// Wait until the nRF goes into Standby-I (or II) mode
			rti_Register(nrf_rtiInitCallback, NULL, RTI_ONCE, RTI_MS_TO_TICKS(NRF_CE_PULSE_DURATION_MS));
						
			break;
			
		case 15:
			// The nRF is now configured and properly initialized
			nrf_isInit = _TRUE;
			ENABLE_IRQ();
						
			break;
	}
}

bool nrf_IsBusy(void)
{
	if (nrf_data.type != PTX)
		err_Throw("nrf: IsBusy is only available in PTX mode.\n");

	return nrf_data.ptxData.transmitting;
}

void nrf_Transmit (u8 *data, u8 length, nrf_PTXptr eot)
{
	if (nrf_data.type != PTX)
		err_Throw("nrf: Transmit can only be called in PTX mode.\n");

	if (nrf_data.ptxData.transmitting == _TRUE)
		err_Throw("nrf: attempted to initiate a transmission while another one is taking place.\n");
	
	if (data == NULL)
		err_Throw("nrf: recevied NULL data pointer.\n");
	
	if ((length == 0) || (length > 32))
		err_Throw("nrf: length must be between 1 and 32.\n");
	
	nrf_data.ptxData.transmitting = _TRUE;
	nrf_data.ptxData.eot = eot;
	
	nrf_CommenceTransmission (data, length);	
}

void nrf_CommenceTransmission (u8 *data, u8 length)
{
	u8 index;
	nrf_data.spiInputData[0] = W_TX_PAYLOAD;
	for (index = 0; index < length; index ++)
		nrf_data.spiInputData[index + 1] = data[index];
		
	spi_Transfer(nrf_data.spiInputData, NULL, length + 1, NULL);
}


void nrf_Receive (nrf_PRXptr eot)
{
	if (nrf_data.type != PRX)
		err_Throw("nrf: Receive can only be called in PRX mode.\n");
	
	if (eot == NULL)
		err_Throw("nrf: received a null end of transmission callback.\n");
	
	if (nrf_data.prxData.eot != NULL)
		err_Throw("nrf: attempted to register a second end of transmission callback.\n");
	
	nrf_data.prxData.eot = eot;	}

void nrf_StoreAckPayload (u8 *data, u8 length)
{	
	u8 index;
	bool intStatus = SafeSei();
	
	if (nrf_data.type != PRX)
		err_Throw("nrf: StoreAckPayload can only be called in PRX mode.\n");
	
	nrf_data.prxData.sendRequest = _TRUE;
		
	if (nrf_data.prxData.receiveRequest == _TRUE)
	{
		nrf_data.prxData.data = data;
		nrf_data.prxData.length = length;
	}
	else
	{
		nrf_data.spiInputData[0] = W_ACK_PAYLOAD(0);
		for (index = 0; index < length; index++)
			nrf_data.spiInputData[index + 1] = data[index];	
			
		spi_Transfer(nrf_data.spiInputData, NULL, length + 1, nrf_StoreAckDone);
	}
	
	SafeCli(intStatus);
}

void nrf_StoreAckDone (void)
{
	nrf_data.prxData.sendRequest = _FALSE;
	if (nrf_data.prxData.receiveRequest == _TRUE)
	{
		nrf_data.spiInputData[0] = NOP;
		spi_Transfer(nrf_data.spiInputData, nrf_data.spiInputData, 1, nrf_spiReadStatusCallback);
	}
}

void interrupt nrf_irq_Service (void)
{
	// The nRF might be issuing an IRQ if it wasn't reset
	if (nrf_isInit != _TRUE)
	{
		DISABLE_IRQ();
		return;
	}
	
	// Since the IRQ won't be serviced until a few SPI transmissions take place, it must be inhibited until then.
	DISABLE_IRQ(); 
	
	// If the PRX is currently uploading an ACK payload, it cannot service the IRQ immediately.
	if (nrf_data.type == PRX)
	{
		nrf_data.prxData.receiveRequest = _TRUE;
		
		if (nrf_data.prxData.sendRequest == _TRUE)			
			return;
	}
	
	// By writing a NOP, the nRF's status register is read, and it is stored in nrf_data.spiInputData[0]
	nrf_data.spiInputData[0] = NOP;
	spi_Transfer(nrf_data.spiInputData, nrf_data.spiInputData, 1, nrf_spiReadStatusCallback);
}

void nrf_spiReadStatusCallback (void)
{
	// Read the status byte, and turn off the corresponding interrupt bits
	if (nrf_data.type == PTX)
	{
		nrf_data.ptxData.callbackStage = 0;
		
		if ((nrf_data.spiInputData[0] & MAX_RT) != 0) // MAX_RT interrupt
		{
			nrf_data.spiInputData[0] = W_REGISTER(STATUS);
			nrf_data.spiInputData[1] = MAX_RT;
			spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_spiHandleMAXRTCallback);
		}
		else // TX_DS interrupt, an RX_DR might have also ocurred if there was payload
		{
			if ((nrf_data.spiInputData[0] & RX_DR) != 0) // TX_DS + RX_DR interrupts
			{
				nrf_data.spiInputData[0] = W_REGISTER(STATUS);
				nrf_data.spiInputData[1] = TX_DS | RX_DR;
				spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_spiHandleTXACKCallback);
			}
			else // TX_DS interrupt
			{
				nrf_data.spiInputData[0] = W_REGISTER(STATUS);
				nrf_data.spiInputData[1] = TX_DS;
				spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_spiHandleTXCallback);
			}
		}
	}
	else // nrf_data.type == PRX
	{
		nrf_data.prxData.callbackStage = 0;
		
		// Either a RX_DR or both RX_DR + TX_DS interrupts have occured - clear both flags just in case
		nrf_data.spiInputData[0] = W_REGISTER(STATUS);
		nrf_data.spiInputData[1] = TX_DS | RX_DR;
		spi_Transfer(nrf_data.spiInputData, NULL, 2, nrf_spiHandleRXCallback);
	}
}

void nrf_spiHandleTXCallback (void)
{
	// TX_DS interrupt - the IRQ has been serviced
	ENABLE_IRQ();
	nrf_data.ptxData.transmitting = _FALSE;
	if (nrf_data.ptxData.eot != NULL)
		nrf_data.ptxData.eot (_TRUE, NULL, 0);
}

void nrf_spiHandleTXACKCallback (void)
{
	switch (nrf_data.ptxData.callbackStage)
	{
		case 0:
			// TX_DS + RX_DR interrupts - the IRQ has been serviced
			ENABLE_IRQ();
			
			nrf_data.ptxData.callbackStage++;
			
			// There's data in the RX FIFO: first, it's width must be read.
			nrf_data.spiInputData[0] = R_RX_PL_WID;
			spi_Transfer(nrf_data.spiInputData, nrf_data.ptxData.readLen, 2, nrf_spiHandleTXACKCallback);
			
			break;
			
		case 1:
			nrf_data.ptxData.callbackStage++;
			
			// nrf_data.ptxData.readLen[1] contains the amount of bytes in the RX FIFO
			nrf_data.spiInputData[0] = R_RX_PAYLOAD;
			spi_Transfer(nrf_data.spiInputData, nrf_data.spiOutputData, nrf_data.ptxData.readLen[1] + 1, nrf_spiHandleTXACKCallback);
			
			break;
			
		case 2:			
			// The data has been read
			nrf_data.ptxData.transmitting = _FALSE;
			if (nrf_data.ptxData.eot != NULL)
				// The first byte on spiOutputData contains the nRF's status register
				nrf_data.ptxData.eot (_TRUE, nrf_data.spiOutputData + 1, nrf_data.ptxData.readLen[1]);
			
			break;
	}
}

void nrf_spiHandleMAXRTCallback (void)
{
	switch (nrf_data.ptxData.callbackStage)
	{
		case 0:
			// MAX_RT interrupt - the IRQ has been serviced
			ENABLE_IRQ();
			
			nrf_data.ptxData.callbackStage++;
			
			// The payload is still in the TX FIFO: it must be flushed out.
			nrf_data.spiInputData[0] = FLUSH_TX;
			spi_Transfer(nrf_data.spiInputData, NULL, 1, nrf_spiHandleMAXRTCallback);
			
			break;
			
		case 1:
		 	// The IRQ has been serviced, and the TX FIFO flushed.
			nrf_data.ptxData.transmitting = _FALSE;
			if (nrf_data.ptxData.eot != NULL)
				nrf_data.ptxData.eot (_FALSE, NULL, 0);
			
			break;
	}
}

void nrf_spiHandleRXCallback (void)
{
	switch (nrf_data.prxData.callbackStage)
	{
		case 0:
			// Either RX_DR or TX_DS + RX_DR interrupts - the IRQ has been serviced
			ENABLE_IRQ();
			
			nrf_data.prxData.callbackStage++;
			
			// There's data in the RX FIFO: first, it's width must be read.
			nrf_data.spiInputData[0] = R_RX_PL_WID;
			spi_Transfer(nrf_data.spiInputData, nrf_data.prxData.readLen, 2, nrf_spiHandleRXCallback);
			
			break;
			
		case 1:
			nrf_data.prxData.callbackStage++;
			
			// nrf_data.prxData.readLen[1] contains the amount of bytes in the RX FIFO
			nrf_data.spiInputData[0] = R_RX_PAYLOAD;
			spi_Transfer(nrf_data.spiInputData, nrf_data.spiOutputData, nrf_data.prxData.readLen[1] + 1, nrf_spiHandleRXCallback);
			
			break;
			
		case 2:			
			// The data has been read
		
			// The first byte on spiOutputData contains the nRF's status register
			nrf_data.prxData.eot (nrf_data.spiOutputData + 1, nrf_data.prxData.readLen[1]);
			
			nrf_data.prxData.receiveRequest = _FALSE;
			if (nrf_data.prxData.sendRequest == _TRUE)
				nrf_StoreAckPayload (nrf_data.prxData.data, nrf_data.prxData.length);
			
			break;
	}
}