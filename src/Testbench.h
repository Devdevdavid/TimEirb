#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include "pmc_interface.h"
#include "tlm_head.h"

// MODULES
#include "Timer.h"

SC_MODULE(Testbench) {
  Timer *timer1;
  tlm_utils::simple_initiator_socket<Testbench> pmcSocket;
  tlm_utils::simple_initiator_socket<Testbench> busSocket;

  SC_CTOR(Testbench);

  void socket_action(tlm_utils::simple_initiator_socket<Testbench> & socket,
                     tlm_generic_payload * trans);
  void pmc_write(const struct pmc_data &pmcData);
  void bus_read(uint32_t address, uint8_t * value, uint8_t length);
  void bus_write(uint32_t address, uint8_t * value, uint8_t length);
  uint8_t bus_read_byte(uint32_t address);
  void bus_write_byte(uint32_t address, uint8_t value);
  void main_test(void);
};

#endif /* _TESTBENCH_H_ */