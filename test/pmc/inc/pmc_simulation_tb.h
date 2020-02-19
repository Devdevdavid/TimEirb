#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"



SC_MODULE(PmcSimulatorTestbench)
{
    tlm_utils::simple_target_socket<PmcSimulatorTestbench> socket;

    SC_CTOR(PmcSimulatorTestbench): socket("PmcSimulatorTestbench")
    {
        socket.register_b_transport(this, &PmcSimulatorTestbench::b_transport);  
    }
	
	virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
	{
		unsigned char*   ptr = trans.get_data_ptr();
		unsigned int     len = trans.get_data_length();
		trans.set_response_status( tlm::TLM_OK_RESPONSE );

        DataClock *data = reinterpret_cast<DataClock *>(ptr);

		cout << "MCK : " << data->mck << " Hz " << "SCLK : " << data->slck << " Hz" << endl;
		
	}
};
