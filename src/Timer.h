#ifndef _TIMER_H_
#define _TIMER_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "systemc.h"

// MODULES
#include "channel.h"

SC_MODULE(Top)
{
	channel *channel;

	SC_CTOR(Timer);
};

#endif /* _TOP_H_ */
