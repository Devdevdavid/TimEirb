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
