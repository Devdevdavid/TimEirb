#ifndef _TIMER_H_
#define _TIMER_H_

#include "tlm_head.h"

// MODULES
SC_MODULE(Timer) {
  SC_CTOR(Timer);
  tlm_utils::simple_target_socket<Timer> pmcSocket;
  tlm_utils::simple_target_socket<Timer> busSocket;
};

#endif /* _TOP_H_ */
