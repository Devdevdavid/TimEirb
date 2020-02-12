#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include "pmc_interface.h"
#include "tlm_head.h"

// MODULES
#include "Timer.h"

SC_MODULE(Testbench) {
/**
 * Public methods
 */
public:
  SC_CTOR(Testbench);

/**
 * Private attributes
 */
private:
  Timer *timer1;
  tlm_utils::simple_initiator_socket<Testbench> pmcSocket;
  tlm_utils::simple_initiator_socket<Testbench> busSocket;

/**
 * SOCKET API
 */
private:
  void socket_action(tlm_utils::simple_initiator_socket<Testbench> & socket,
                     tlm_generic_payload * trans);
  int pmc_write(const struct pmc_data &pmcData);

  int bus_read(uint32_t address, uint8_t * value, uint8_t length);
  int bus_write(uint32_t address, uint8_t * value, uint8_t length);

  int bus_read_byte(uint32_t address, uint32_t *value);
  int bus_write_byte(uint32_t address, uint32_t value);

  int timer0_read_byte(uint32_t address, uint32_t *value);
  int timer0_write_byte(uint32_t address, uint32_t value);

/**
 * FUNCTIONNAL API
 */
public:
  int set_pmc_data(uint32_t mck, uint32_t slck);
  int set_write_protection(bool isEnabled);

/**
 * TEST API
 */
public:
  int test_timer_address(void);
  int test_write_protection(void);
  int test_interruption(void);
  int test_write_register_ABC(void);

/**
 * TEST STEP
 */
public:
  void main_test(void);
};

#endif /* _TESTBENCH_H_ */