#include "nRF24L01+.h"
#include "error.h"
#include "rti.h"

// SPI COMMANDS
#define R_REGISTER(x) (x)
#define W_REGISTER(x) (BIT(5) & x)

#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2

#define REUSE_TX_PL 0xE3
#define R_RX_PL_WID 0x60
#define W_ACK_PAYLOAD(x) (0xA8 & x)
#define W_TX_PAYLOAD_NO_ACK 0xB0

#define NOP 0xFF

// REGISTER ADDRESSES & BITS
#define CONFIG 0x00
#define MASK_RX_DR BIT(6)
#define MASK_TX_DS BIT(5)
#define MASK_MAX_RT BIT(4)
#define EN_CRC BIT(3)
#define CRCO BIT(2)
#define CRC_1_BYTE 0
#define CRC_2_BYTE 1
#define PWR_UP BIT(1)
#define PRIM_RX BIT(0)

#define EN_AA 0x01 // Enable Auto Acknowledgement
#define ENAA_P(x) BIT(x)

#define EN_RXADDR 0x02 // Enable RX Addresses
#define ERX_P(x) BIT(x)

#define SETUP_AW 0x03 // Setup of Address Width
#define AW_3_BYTES 0x01
#define AW_4_BYTES 0x10
#define AW_5_BYTES 0x11

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
#define RF_PWR_m6DBM ((0x10) << 1)
#define RF_PWR_0DBM ((0x11) << 1)

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

#define DYNDP 0x1C
#define DPL_P(x) BIT(x)  // Dynamic Payload Length for pipe x

#define FEATURE 0x1D
#define EN_DPL BIT(2) // Enable Dynamic Payload Length
#define EN_ACK_PAY BIT(1) // Enable Payload with ACK
#define EN_DYN_ACK BIT(0) // Enable the W_TX_PAYLOAD_NO_ACK command

typedef struct 
{
	u8 dummy;
} nrf_transferData;


struct {
	nrf_Type type;
} nrf_data;

bool nrf_isInit = _FALSE;

void nrf_Init (nrf_Type type)
{
	if (nrf_isInit == _TRUE)
		return;
	
	nrf_isInit = _TRUE;
	
	nrf_data.type = type;
	
	rti_Init();
	// init sequence - wait times, configure ALL registers, depending on wether TX or RX
	
	// RX: callback para un UNICO pipe. me da un chorizo de memoria, y cuando lo llamo ahí está lo que mandaron
	// , y le digo el largo. Funcion para guardar el ack payload.	
	
	// TX: funcion para mandar. Chequeo de no llenar la FIFO (solo esta todo bien si la fifo no está llena). Me pasas
	// chorizo de memoria y largo, yo mando eso. te llamo cuando termino y recibi un ack, o cuando hubo un error. Si
	// llego un ack, te paso el payload, si hay.
	
	return;
}

void interrupt nrf_irq_Service (void)
{
	
}