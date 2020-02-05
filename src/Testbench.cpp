#include "Testbench.h"

Testbench::Testbench(sc_module_name name)
    : sc_module(name), pmcSocket("pmcSocket"), busSocket("busSocket") {
  timer1 = new Timer("Timer1");

  pmcSocket.bind(timer1->socket_PMC);
  busSocket.bind(timer1->socket_Bus);

  SC_THREAD(main_test);
}

void Testbench::socket_action(
    tlm_utils::simple_initiator_socket<Testbench> &socket,
    tlm_generic_payload *trans) {
  sc_time delay = sc_time(10, SC_NS);

  trans->set_byte_enable_ptr(0);
  trans->set_dmi_allowed(false);
  trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

  // Send it !
  socket->b_transport(*trans, delay);
}

void Testbench::pmc_write(const struct pmc_data &pmcData) {
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_command(TLM_WRITE_COMMAND);
  trans->set_address(0);
  trans->set_data_ptr((uint8_t *)&pmcData);
  trans->set_data_length(sizeof(struct pmc_data));

  socket_action(pmcSocket, trans);

  if (trans->is_response_error()) {
    SC_REPORT_ERROR("Testbench::pmc_write()", "PMC Write Error");
  }

  delete (trans);
}
/** no pmc_read() */

void Testbench::bus_read(uint32_t address, uint8_t *value, uint8_t length) {
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_command(TLM_READ_COMMAND);
  trans->set_address(address);
  trans->set_data_ptr(value);
  trans->set_data_length(length);

  socket_action(busSocket, trans);

  if (trans->is_response_error()) {
    SC_REPORT_ERROR("Testbench::bus_read()", "Bus Read Error");
  }

  delete (trans);
}

void Testbench::bus_write(uint32_t address, uint8_t *value, uint8_t length) {
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_command(TLM_WRITE_COMMAND);
  trans->set_address(address);
  trans->set_data_ptr(value);
  trans->set_data_length(length);

  socket_action(busSocket, trans);

  if (trans->is_response_error()) {
    SC_REPORT_ERROR("Testbench::bus_write()", "Bus Write Error");
  }

  delete (trans);
}

uint8_t Testbench::bus_read_byte(uint32_t address) {
  uint8_t value;

  bus_read(address, &value, 1);

  return value;
}

void Testbench::bus_write_byte(uint32_t address, uint8_t value) {
  bus_write(address, &value, 1);
}

void Testbench::main_test(void) {
  struct pmc_data pmcData;
  pmcData.mck = 10;
  pmcData.slck = 20;
  pmc_write(pmcData);
  bus_write_byte(1, 1);
}
