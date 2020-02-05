#include "Testbench.h"
#include "Tools.h"

Testbench::Testbench(sc_module_name name)
    : sc_module(name), pmcSocket("pmcSocket"), busSocket("busSocket") {
  timer1 = new Timer("Timer1", TIMER0_BASE_ADDR);

  pmcSocket.bind(timer1->socket_PMC);
  busSocket.bind(timer1->socket_Bus);

  SC_THREAD(main_test);
}

/********************************************************
 *						SOCKET API
 ********************************************************/

void Testbench::socket_action(
    tlm_utils::simple_initiator_socket<Testbench> &socket,
    tlm_generic_payload *trans) {
  sc_time delay = sc_time(10, SC_NS);

  trans->set_byte_enable_ptr(0);
  trans->set_dmi_allowed(false);
  trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

  // Send it !
  socket->b_transport(*trans, delay);

  // Print error condition
  if (trans->is_response_error()) {
  	char rwChar = (trans->get_command() == TLM_READ_COMMAND) ? 'R' : 'W';
    fprintf(stderr, "Testbench::socket_action() %c@0x%04X L=%dB\n",
    	rwChar,
    	trans->get_address(),
    	trans->get_data_length());
  }
}

int Testbench::pmc_write(const struct pmc_data &pmcData) {
  int ret = 0;
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_command(TLM_WRITE_COMMAND);
  trans->set_address(0);
  trans->set_data_ptr((uint8_t *)&pmcData);
  trans->set_data_length(sizeof(struct pmc_data));

  socket_action(pmcSocket, trans);

  if (trans->is_response_error()) {
    fprintf(stderr, "Testbench::pmc_write() Transaction failed: %s\n", trans->get_response_string().c_str());
    ret = -1;
  }

  delete (trans);
  return ret;
}
/** no pmc_read() */

int Testbench::bus_read(uint32_t address, uint8_t *value, uint8_t length) {
  int ret = 0;
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_command(TLM_READ_COMMAND);
  trans->set_address(address);
  trans->set_data_ptr(value);
  trans->set_data_length(length);

  socket_action(busSocket, trans);

  if (trans->is_response_error()) {
    fprintf(stderr, "Testbench::bus_read() Transaction failed: %s\n", trans->get_response_string().c_str());
    ret = -1;
  }

  delete (trans);
  return ret;
}

int Testbench::bus_write(uint32_t address, uint8_t *value, uint8_t length) {
  int ret = 0;
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_command(TLM_WRITE_COMMAND);
  trans->set_address(address);
  trans->set_data_ptr(value);
  trans->set_data_length(length);

  socket_action(busSocket, trans);

  if (trans->is_response_error()) {
    fprintf(stderr, "Testbench::bus_write() Transaction failed: %s\n", trans->get_response_string().c_str());
    ret = -1;
  }

  delete (trans);
  return ret;
}

int Testbench::bus_read_byte(uint32_t address, uint8_t *value) {
  return bus_read(address, value, 1);
}

int Testbench::bus_write_byte(uint32_t address, uint8_t value) {
  return bus_write(address, &value, 1);
}

/********************************************************
 *						TEST API
 ********************************************************/

int Testbench::set_pmc_data_valid(uint32_t mck, uint32_t slck) {
  struct pmc_data pmcData;
  pmcData.mck = mck;
  pmcData.slck = slck;

  if (pmc_write(pmcData)) {
	SC_REPORT_ERROR("Testbench::set_pmc_data_valid()", "Unable to set PMC clocks");
  }
  return 0;
}

int Testbench::test_timer_address(void) {
  if (bus_write_byte(TIMER0_BASE_ADDR, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@0x0");
  }
  if (bus_write_byte(TIMER0_BASE_ADDR + TIMER_CHANNEL_ADDR_SPACE, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@CH0@0x0");
  }
  if (bus_write_byte(TIMER0_BASE_ADDR + 2 * TIMER_CHANNEL_ADDR_SPACE, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@CH1@0x0");
  }
  if (bus_write_byte(TIMER0_BASE_ADDR + 3 * TIMER_CHANNEL_ADDR_SPACE, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@CH2@0x0");
  }
  if (bus_write_byte(TIMER0_BASE_ADDR + TIMER_ADDR_SPACE, 0) == 0) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Can write at T0@_ and shouldn't be possible");
  }

  return 0;
}

/********************************************************
 *						TEST STEPS
 ********************************************************/

void Testbench::main_test(void) {
  set_pmc_data_valid(0, 0);
  set_pmc_data_valid(10 * MEGA, 32 * KILO);
  test_timer_address();
}
