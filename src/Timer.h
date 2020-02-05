#ifndef _TIMER_H_
#define _TIMER_H_

#include "tlm_head.h"

// MODULES
#include "channel.h"

SC_MODULE(Timer)
{

  Channel *channel1;

  tlm_utils::simple_target_socket<Timer> socket_PMC;
  tlm_utils::simple_target_socket<Timer> socket_Bus;

	SC_CTOR(Timer);
void Timer::b_transport_pcm(tlm_generic_payload& trans, sc_time& delay)
void Timer::b_transport_bus(tlm_generic_payload& trans, sc_time& delay)

};

#endif /* _TIER_H_ */
