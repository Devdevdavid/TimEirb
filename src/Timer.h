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
#define TC_BCR	0xC0
#define TC_BMR	0xC4
#define TC_QIER	0xC8
#define TC_QIDR	0xCC
#define TC_QIMR	0xD0
#define TC_QISR	0xD4
#define TC_FMR	0xD8
#define TC_WPMR	0xE4

/** TC_BCR bit definition */
#define TC_BCR_SYNC	(1 << 0)

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
	int set_register(uint8_t cmd, uint32_t address, uint32_t value);
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
	struct pmc_data curPmcData;						/** Last data received from PMC module */
};

#endif /* _TIER_H_ */
