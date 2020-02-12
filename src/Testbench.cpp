#include "Testbench.h"
#include "Tools.h"

Testbench::Testbench(sc_module_name name) : sc_module(name), pmcSocket("pmcSocket"), busSocket("busSocket") {
  timer1 = new Timer("Timer1", TIMER0_BASE_ADDR);

  pmcSocket.bind(timer1->socket_PMC);
  busSocket.bind(timer1->socket_Bus);

  SC_THREAD(main_test);
}

/********************************************************
 *						SOCKET API
 ********************************************************/

void Testbench::socket_action(tlm_utils::simple_initiator_socket<Testbench> &socket, tlm_generic_payload *trans) {
  sc_time delay = sc_time(10, SC_NS);

  trans->set_byte_enable_ptr(0);
  trans->set_dmi_allowed(false);
  trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

  // Send it !
  socket->b_transport(*trans, delay);

  // Print error condition
  if (trans->is_response_error()) {
  	char rwChar = (trans->get_command() == TLM_READ_COMMAND) ? 'R' : 'W';
    //fprintf(stderr, "Testbench::socket_action() %c@0x%04X L=%dB\n", rwChar, trans->get_address(), trans->get_data_length());
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
    //fprintf(stderr, "Testbench::pmc_write() Transaction failed: %s\n", trans->get_response_string().c_str());
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
    //fprintf(stderr, "Testbench::bus_read() Transaction failed: %s\n", trans->get_response_string().c_str());
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
    //fprintf(stderr, "Testbench::bus_write() Transaction failed: %s\n", trans->get_response_string().c_str());
    ret = -1;
  }

  delete (trans);
  return ret;
}

int Testbench::bus_read_byte(uint32_t address, uint32_t *value) {
  return bus_read(address, (uint8_t *) value, sizeof(*value));
}

int Testbench::bus_write_byte(uint32_t address, uint32_t value) {
  return bus_write(address, (uint8_t *) &value, sizeof(value));
}

int Testbench::timer0_read_byte(uint32_t address, uint32_t *value) {
  return bus_read_byte(TIMER0_BASE_ADDR + address, value);
}

int Testbench::timer0_write_byte(uint32_t address, uint32_t value) {
  return bus_write_byte(TIMER0_BASE_ADDR + address, value);
}

/********************************************************
 *            FUNCTIONNAL API
 ********************************************************/

int Testbench::set_pmc_data(uint32_t mck, uint32_t slck)
{
  struct pmc_data pmcData;
  pmcData.mck = mck;
  pmcData.slck = slck;

  if (pmc_write(pmcData)) {
    SC_REPORT_ERROR("Testbench::set_pmc_data_valid()", "Unable to set PMC clocks");
  }
  return 0;
}

int Testbench::set_write_protection(bool isEnabled)
{
  uint32_t value = (isEnabled) ? TC_WPMR_WPEN : 0;
  value |= (TC_WPMR_PASSWORD << 8);
  return timer0_write_byte(TC_WPMR, value);
}

/********************************************************
 *						TEST API
 ********************************************************/

int Testbench::test_timer_address(void) {
  printf("> BEGIN TIMER ADDRESS\n");

  if (timer0_write_byte(0, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@0x0");
  }
  if (timer0_write_byte(TIMER_CHANNEL_ADDR_SPACE, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@CH0@0x0");
  }
  if (timer0_write_byte(2 * TIMER_CHANNEL_ADDR_SPACE, 0)) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@CH1@0x0");
  }
  if (timer0_write_byte(TIMER_ADDR_SPACE, 0) == 0) {
  	SC_REPORT_ERROR("Testbench::test_timer_address()", "Can write at T0@_ and shouldn't be possible");
  }

  // Test timer itself
  if (timer0_write_byte(TC_BCR, 0)) {
    SC_REPORT_ERROR("Testbench::test_timer_address()", "Unable to write at T0@BCR");
  }
  if (timer0_write_byte(TC_BCR + 1, 0) == 0) {
    SC_REPORT_ERROR("Testbench::test_timer_address()", "Can write at T0@BCR+1 and shouldn't be possible");
  }
  if (timer0_write_byte(TC_QIMR, 0) == 0) {
    SC_REPORT_ERROR("Testbench::test_timer_address()", "Can write at T0@QIMR and should be read only");
  }

  printf("> TIMER ADDRESS: PASSED\n");
  return 0;
}

int Testbench::test_write_protection(void)
{
  uint32_t tmp1 = TC_BMR_Mask, tmp2 = 0;

  printf("> BEGIN WRITE PROTECTION\n");

  if (timer0_write_byte(TC_BMR, tmp1)) {
    SC_REPORT_ERROR("Testbench::test_write_protection()", "Unable to write at T0@BMR");
  }
  if (set_write_protection(true)) {
    SC_REPORT_ERROR("Testbench::test_write_protection()", "Can't turn on write protection");
  }
  if (timer0_write_byte(TC_BMR, 0xBB66) == 0) {
    SC_REPORT_ERROR("Testbench::test_write_protection()", "Can write at T0@BMR and should be protected");
  }
  if (set_write_protection(false)) {
    SC_REPORT_ERROR("Testbench::test_write_protection()", "Can't turn off write protection");
  }
  if (timer0_read_byte(TC_BMR, &tmp2)) {
    SC_REPORT_ERROR("Testbench::test_write_protection()", "Unable to read T0@BMR");
  }

  if (tmp1 != tmp2) {
    SC_REPORT_ERROR("Testbench::test_write_protection()", "Value of T0@BMR have been altred during write protection");
  }

  printf("> WRITE PROTECTION: PASSED\n");
  return 0;
}

int Testbench::test_interruption(void)
{
  uint32_t tmp;
  uint32_t interTimerArray[] = {TC_QIxR_IDX, TC_QIxR_DIRCHG, TC_QIxR_QERR};
  uint32_t interChannelArray[] = {TC_IxR_COVFS, TC_IxR_LOVRS, TC_IxR_CPAS, TC_IxR_CPBS, TC_IxR_CPCS, TC_IxR_LDRAS, TC_IxR_LDRBS, TC_IxR_ETRGS};

  printf("> BEGIN INTERRUPTION\n");

  for (int i = 0; i < sizeof(interTimerArray) / sizeof(uint32_t); ++i) {
    // Enable interrupt
    if (timer0_write_byte(TC_QIER, interTimerArray[i])) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@QIER");
    }
    // Read mask
    if (timer0_read_byte(TC_QIMR, &tmp)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to read at T0@QIMR");
    }
    // Test value
    if (tmp & interTimerArray[i] == 0) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to set the interruption on T0");
    }
    // Disable interrupt
    if (timer0_write_byte(TC_QIDR, interTimerArray[i])) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@QIDR");
    }
    // Read mask
    if (timer0_read_byte(TC_QIMR, &tmp)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to read at T0@QIMR");
    }
    // Test value
    if (tmp & interTimerArray[i] != 0) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to reset the interruption on T0");
    }
  }
  printf("Timer interuption OK\n");

  for (int i = 0; i < sizeof(interChannelArray) / sizeof(uint32_t); ++i) {
    // Enable interrupt
    if (timer0_write_byte(TC_IER, interChannelArray[i])) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@CH0@IER");
    }
    // Read mask
    if (timer0_read_byte(TC_IMR, &tmp)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to read at T0@CH0@IMR");
    }
    // Test value
    if (tmp & interChannelArray[i] == 0) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to set the interruption on T0@CH0");
    }
    // Disable interrupt
    if (timer0_write_byte(TC_IDR, interChannelArray[i])) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@CH0@IDR");
    }
    // Read mask
    if (timer0_read_byte(TC_IMR, &tmp)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to read at T0@CH0@IMR");
    }
    // Test value
    if (tmp & interChannelArray[i] != 0) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to reset the interruption on T0@CH0");
    }
  }
  printf("Channel interuption OK\n");

  printf("> INTERRUPTION: PASSED\n");
  return 0;
}

int Testbench::test_write_register_ABC(void)
{
  uint32_t regABCArray[] = {TC_RA, TC_RB, TC_RC};
  uint32_t tmp1 = 0x5A5A, tmp2;

  printf("> BEGIN WRITE REGISTER A/B/C\n");

  for (int i = 0; i < sizeof(regABCArray) / sizeof(uint32_t); ++i) {
    printf("TC_R%c:\n", (i == 0) ? 'A' : (i == 1) ? 'B' : 'C');

    if (set_write_protection(false)) {
      SC_REPORT_ERROR("Testbench::test_write_protection()", "Can't turn off write protection");
    }
    if (timer0_write_byte(TC_CMR, TC_CMRx_WAVE)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@CH0@CMR");
    }
    if (timer0_write_byte(regABCArray[i], tmp1)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@CH0@regA/B/C");
    }
    if (timer0_write_byte(TC_CMR, 0)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to write at T0@CH0@CMR");
    }
    if (set_write_protection(true)) {
      SC_REPORT_ERROR("Testbench::test_write_protection()", "Can't turn on write protection");
    }
    // Try to corrupt
    if (timer0_write_byte(regABCArray[i], 0xA0A0) == 0) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Can write at T0@CH0@regA/B/C and should be protected");
    }
    // Read data and check
    if (timer0_read_byte(regABCArray[i], &tmp2)) {
      SC_REPORT_ERROR("Testbench::test_interruption()", "Unable to read at T0@CH0@regA/B/C");
    }

    if (tmp1 != tmp2) {
      SC_REPORT_ERROR("Testbench::test_write_protection()", "Value of T0@CH0@regA/B/C have been altred during write protection");
    }
  }

  printf("> WRITE REGISTER A/B/C: PASSED\n");
  return 0;
}

/********************************************************
 *						TEST STEPS
 ********************************************************/

void Testbench::main_test(void) {
  set_pmc_data(0, 0);
  set_pmc_data(10 * MEGA, 32 * KILO);
  test_timer_address();
  test_write_protection();
  test_interruption();
  test_write_register_ABC();
}
