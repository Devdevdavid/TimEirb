#ifndef _INITIATOR_H_
#define _INITIATOR_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
using namespace sc_core;
using namespace sc_dt;
using namespace std;
using namespace tlm;

#include "systemc.h"

SC_MODULE(Initiator)
{
	tlm_utils::simple_initiator_socket<Initiator> socket;

	SC_CTOR(Initiator);
	void thread_process(void);
	void print_buffer(uint8_t data[], uint16_t len);
};

#endif /* _INITIATOR_H_ */