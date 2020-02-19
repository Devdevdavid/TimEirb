#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"


struct DataClock {
    uint32_t mck;
    uint32_t slck;
};


SC_MODULE(PmcSimulator)
{
    tlm_utils::simple_initiator_socket<PmcSimulator> pmcSocket;
    
    SC_CTOR(PmcSimulator): 
    	pmcSocket("PMCSimulator")
    {
        SC_THREAD(thread_process);
    }

    void thread_process()
    {
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
		sc_time delay = sc_time(10, SC_NS);
        DataClock data = {
            .mck = 20'000'000,
            .slck = 32'768
        };
        trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
        trans->set_data_length(sizeof(data));
        trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
        pmcSocket->b_transport( *trans, delay );
        
        if (trans->is_response_error())
	  			SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        free(trans);
    }
};
