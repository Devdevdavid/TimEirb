#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "pmc.h"
#include "data_clock.h"

SC_MODULE(Testbench)
{
    tlm_utils::simple_initiator_socket<Testbench> configSocket;
    tlm_utils::simple_target_socket<Testbench> UartSocket;
	tlm_utils::simple_target_socket<Testbench> GpioSocket;
	tlm_utils::simple_target_socket<Testbench> TimerSocket;
    int transactionNumber;

    SC_HAS_PROCESS(Testbench);
    Testbench(sc_module_name name);
    
    virtual void UART_b_transport(tlm::tlm_generic_payload &trans, sc_time &delay);
    virtual void GPIO_b_transport(tlm::tlm_generic_payload &trans, sc_time &delay);
    virtual void TIMER_b_transport(tlm::tlm_generic_payload &trans, sc_time &delay);
    void run_test();
    //void testTransaction( tlm::tlm_generic_payload *trans, uint32_t expectedMck, uint32_t expectedSclk);
    void testUartTransaction( tlm::tlm_generic_payload *trans, uint32_t expectedMck, uint32_t expectedSclk);
    void testTimerTransaction( tlm::tlm_generic_payload *trans, uint32_t expectedMck, uint32_t expectedSclk);
    void testGpioTransaction( tlm::tlm_generic_payload *trans, uint32_t expectedMck, uint32_t expectedSclk);
    void sendTransactionConfigSocket(tlm::tlm_generic_payload *trans);
    DataClock *parseTransactionData(tlm::tlm_generic_payload *trans);
private:
    //DataClock expectedClock;
    DataClock expectedUartClock;
    DataClock expectedTimerClock;
    DataClock expectedGpioClock;

};
