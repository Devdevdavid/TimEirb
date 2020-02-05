#ifndef _TOP_H_
#define _TOP_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "systemc.h"

// MODULES
#include "Initiator.h"
#include "Memory.h"

SC_MODULE(Top)
{
	Initiator *initiator;
	Memory    *memory;

	SC_CTOR(Top);
};

#endif /* _TOP_H_ */