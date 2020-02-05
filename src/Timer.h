#ifndef _TIMER_H_
#define _TIMER_H_

#include "tlm_head.h"

// MODULES
#include "Channel.h"

SC_MODULE(Timer)
{
  tlm_utils::simple_timer_socket<Timer> socket_PMC;
  tlm_utils::simple_timer_socket<Timer> socket_Bus;

	channel *channel;

	SC_CTOR(Timer);
};

#endif /* _TIER_H_ */
