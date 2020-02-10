#ifndef _TIMER_H_
#define _TIMER_H_

#include "tlm_head.h"
#include "pmc_interface.h"

// MODULES
#include "Channel.h"

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
#define TC_BCR_SYNC	(1 << 0)

/** TC_WPMR bit definition */
#define TC_WPMR_PASSWORD	0x54494D
#define TC_WPMR_WPEN		(1 << 0)

SC_MODULE(Timer)
{
/**
 * Public methods
 */
public:
	Timer(sc_module_name name, uint32_t baseAddress);
	void b_transport_pcm(tlm_generic_payload& trans, sc_time& delay);
	void b_transport_bus(tlm_generic_payload& trans, sc_time& delay);

/**
 * Private methods
 */
private:
	int manage_register(uint8_t cmd, uint32_t address, uint32_t *pData);
	void set_write_protection(bool isEnabled);

/**
 * Public attributes
 */
public:
	tlm_utils::simple_target_socket<Timer> socket_PMC;
	tlm_utils::simple_target_socket<Timer> socket_Bus;

/**
 * Private attributes
 */
private:
	uint32_t baseAddress;
	Channel *channel1;
	struct pmc_data curPmcData;					/** Last data received from PMC module */
	uint32_t registerData[TC_REG_COUNT];		/** Value of all the internal regsiters */
	bool isWriteProtected;						/** Tell if Write protection is enabled (Works on some registers) */
};

#endif /* _TIER_H_ */
