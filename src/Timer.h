#ifndef _TIMER_H_
#define _TIMER_H_

#include <vector>

#include "tlm_head.h"
#include "pmc_interface.h"

// MODULES
#include "Channel.h"

// CONSTANTS
#define CHANNEL_COUNT		3

// ADDRESSES
#define TIMER0_BASE_ADDR	0x40080000
#define TIMER1_BASE_ADDR	0x40084000
#define TIMER2_BASE_ADDR	0x40088000

#define TIMER_ADDR_SPACE	0xE8

// RESGISTERS ADDRESS TO TIMER ITSELF
#define TC_BCR				0xC0
#define TC_BMR				0xC4
#define TC_QIER				0xC8
#define TC_QIDR				0xCC
#define TC_QIMR				0xD0
#define TC_QISR				0xD4
#define TC_FMR				0xD8
#define TC_WPMR				0xE4

// REGISTER INDEXES
#define TC_BCR_I 			0
#define TC_BMR_I 			1
#define TC_QIER_I 			2
#define TC_QIDR_I 			3
#define TC_QIMR_I 			4
#define TC_QISR_I 			5
#define TC_FMR_I 			6
#define TC_WPMR_I 			7
#define TC_REG_COUNT 		8

/** TC_BCR bit definition */
#define TC_BCR_SYNC			(1 << 0)

/** TC_BMR bit definition */
#define TC_BMR_TC0XC0S		(3 << 0)
#define TC_BMR_TC1XC1S		(3 << 2)
#define TC_BMR_TC2XC2S		(3 << 4)
#define TC_BMR_QDEN			(1 << 8)
#define TC_BMR_POSEN		(1 << 9)
#define TC_BMR_SPEEDEN		(1 << 10)
#define TC_BMR_QDTRANS		(1 << 11)
#define TC_BMR_EDGPHA		(1 << 12)
#define TC_BMR_INVA			(1 << 13)
#define TC_BMR_INVB			(1 << 14)
#define TC_BMR_INVIDX		(1 << 15)
#define TC_BMR_SWAP			(1 << 16)
#define TC_BMR_IDXPHB		(1 << 17)
#define TC_BMR_MAXFILT		(0x3F << 20)
#define TC_BMR_Mask			(0x03F3FF3F)

/** TC_QIER bit definition */
/** TC_QIDR bit definition */
/** TC_QIMR bit definition */
/** TC_QISR bit definition */
#define TC_QIxR_IDX			(1 << 0)
#define TC_QIxR_DIRCHG		(1 << 1)
#define TC_QIxR_QERR		(1 << 2)
#define TC_QIxR_Mask		(TC_QIxR_IDX | TC_QIxR_DIRCHG | TC_QIxR_QERR)

/** TC_FMR bit definition */
#define TC_FMR_ENCF0		(1 << 0)
#define TC_FMR_ENCF1		(1 << 1)
#define TC_FMR_Mask			(TC_FMR_ENCF0 | TC_FMR_ENCF1)

/** TC_WPMR bit definition */
#define TC_WPMR_PASSWORD	0x54494D
#define TC_WPMR_WPEN		(1 << 0)

/** TC_CCR bit definition */
#define TC_CCR0		(1 << 0)



SC_MODULE(Timer)
{
/**
 * Public methods
 */
public:
	SC_CTOR(Timer);
	void b_transport_pcm(tlm_generic_payload& trans, sc_time& delay);
	void b_transport_bus(tlm_generic_payload& trans, sc_time& delay);
	void set_base_address(uint32_t baseAddress);

/**
 * Private methods
 */
private:
	int manage_register(uint8_t cmd, uint32_t address, uint32_t *pData);
	void set_write_protection(bool isEnabled);
	void set_clock_enable(uint8_t channelId, bool isEnabled);

/**
 * Public attributes
 */
public:
	tlm_utils::simple_target_socket<Timer> socketPMC;
	tlm_utils::simple_target_socket<Timer> socketBus;
	Channel *channels[CHANNEL_COUNT];					/** Array of channels */

/**
 * Private attributes
 */
private:
	uint32_t baseAddress;
	struct pmc_data curPmcData;					/** Last data received from PMC module */
	uint32_t registerData[TC_REG_COUNT];		/** Value of all the internal regsiters */
	bool isWriteProtected;						/** Tell if Write protection is enabled (Works on some registers) */
};

#endif /* _TIER_H_ */
