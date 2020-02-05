#ifndef _MEMORY_H_
#define _MEMORY_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
using namespace sc_core;
using namespace sc_dt;
using namespace std;
using namespace tlm;

#include "systemc.h"

// CONSTANTS
#define MEMORY_MAX_SIZE 128

SC_MODULE(Memory)
{
	uint8_t mem[MEMORY_MAX_SIZE];
	tlm_utils::simple_target_socket<Memory> socket;

	SC_CTOR(Memory);
	void b_transport(tlm_generic_payload& trans, sc_time& delay);
};

#endif /* _MEMORY_H_ */