#ifndef _TIMER_H_
#define _TIMER_H_

#include "tlm_head.h"

// MODULES
#include "Channel.h"

SC_MODULE(Timer) {
  tlm_utils::simple_target_socket<Timer> socket_PMC;
  tlm_utils::simple_target_socket<Timer> socket_Bus;

  Channel *channel;

  void b_transport_pcm(tlm_generic_payload & trans, sc_time & delay);
  void b_transport_bus(tlm_generic_payload & trans, sc_time & delay);

  SC_CTOR(Timer);
};

#endif /* _TIER_H_ */
