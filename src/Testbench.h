#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include "pmc_interface.h"
#include "tlm_head.h"
#include "pmc_registers.h"

#define     PMC_BASE_ADDR       0x400e0600
#define     PMC_MCKR_ADDR       (PMC_BASE_ADDR + 0x30)

// MODULES
//#include "pmc_simulator.h"
#include "Timer.h"

SC_MODULE(Testbench) {
/**
 * Public methods
 */
public:
  Timer *timer1;
  tlm_utils::simple_initiator_socket<Testbench> configSocket;
  SC_CTOR(Testbench);

/**
 * Private attributes
 */
private:
  //PmcSimulator *pmcSimulator;
  
  tlm_utils::simple_initiator_socket<Testbench> busSocket;
  tlm_utils::simple_target_socket<Testbench> tioSockets[CHANNEL_COUNT];

  struct socket_tio_data_t tioData[CHANNEL_COUNT];

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
 * PIO MANAGEMENT
 */
void b_transport_tio(tlm_generic_payload& trans, sc_time& delay);

/**
 * FUNCTIONNAL API
 */
public:
  int set_pmc_data(uint32_t mck, uint32_t slck);
  int set_write_protection(bool isEnabled);
  int set_clock_enable(uint8_t channelId, bool isEnabled);

/**
 * TEST API
 */
public:
  int test_timer_configuration(void);
  int test_timer_address(void);
  int test_write_protection(void);
  int test_interruption(void);
  int test_write_register_ABC(void);
  int test_counter_update(void);
  int test_tio_ab(void);

/**
 * TEST STEP
 */
public:
  void main_test(void);
};

#endif /* _TESTBENCH_H_ */