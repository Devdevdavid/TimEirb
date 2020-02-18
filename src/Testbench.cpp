#include "Testbench.h"
#include "Tools.h"

Testbench::Testbench(sc_module_name name) : sc_module(name), pmcSocket("pmcSocket"), busSocket("busSocket") {
  timer1 = new Timer("Timer1");
  timer1->set_base_address(TIMER0_BASE_ADDR);

  pmcSocket.bind(timer1->socketPMC);
  busSocket.bind(timer1->socketBus);

  for (int i = 0; i < CHANNEL_COUNT; ++i) {
    timer1->channels[i]->tioSocket.bind(tioSockets[i]);
    tioSockets[i].register_b_transport(this, &Testbench::b_transport_tio);
  }

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
 *            PIO MANAGEMENT
 ********************************************************/

void Testbench::b_transport_tio(tlm_generic_payload& trans, sc_time& delay)
{
    uint8_t channelId = trans.get_address();
    struct socket_tio_data_t *tioData;

    // The only data valid is a struct pmc_data, check length
    if (trans.get_data_length() != sizeof(struct socket_tio_data_t)) {
      trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
      return;
    }

    // Cast and check the pointer
    tioData = (struct socket_tio_data_t *) trans.get_data_ptr();
    if (tioData == NULL) {
      trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
      return;
    }

    // Check channel Id
    if (channelId >= CHANNEL_COUNT) {
      trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
      return;
    }

    // Save the given value
    memcpy(&(this->tioData[channelId]), tioData, sizeof(struct socket_tio_data_t));

    cout << "New TIO value for channel " << to_string(channelId) << endl;
    cout << "TIOA Freq = " << this->tioData[channelId].tioA.clockFrequency << endl;
    cout << "TIOA Duty = " << this->tioData[channelId].tioA.dutyCycle << endl;
    cout << "TIOB Freq = " << this->tioData[channelId].tioB.clockFrequency << endl;
    cout << "TIOB Duty = " << this->tioData[channelId].tioB.dutyCycle << endl;

    trans.set_response_status(TLM_OK_RESPONSE);
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

int Testbench::set_clock_enable(uint8_t channelId, bool isEnabled)
{
  uint32_t value = (isEnabled) ? TC_CCR_CLKEN : TC_CCR_CLKDIS;

  return timer0_write_byte(channelId * TIMER_CHANNEL_ADDR_SPACE + TC_CCR, value);
}

/********************************************************
 *						TEST API
 ********************************************************/


int Testbench::test_timer_configuration(void)
{
  uint32_t tmp;
  uint32_t tmpArray[] = {0, 10, 100};
  uint32_t regABCArray[] = {TC_RA, TC_RB, TC_RC};

  printf("> BEGIN TIMER CONFIGURATION\n");


  // clock configuration
  if (set_clock_enable(0, false)) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Can't disable the clock");
  }
  // Read mask
  if (timer0_read_byte(TC_SR, &tmp)) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to read at T0@SR0");
  }
  // Test value
  if (tmp & TC_SR_CLKSTA != 1) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to reset the clock on T0");
  }

  if (set_clock_enable(0, true)) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Can't enable the clock");
  }
  // Read mask
  if (timer0_read_byte(TC_SR, &tmp)) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to read at T0@SR0");
  }
  // Test value
  if (tmp & TC_SR_CLKSTA != 0) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to set the clock on T0");
  }

  // mode configuration
  if (timer0_write_byte(TC_CMR, TC_CMRx_WAVE)) {
      SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to write at T0@CH0@CMR");
  }
  // Read mask
  if (timer0_read_byte(TC_CMR, &tmp)) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to read at T0@CMR");
  }
  // Test value
  if (tmp & TC_CMRx_WAVE != 0) {
    SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to set waveform mode T0");
  }


  // RA, RB and RC configuration
  for (int i = 0; i < sizeof(regABCArray) / sizeof(uint32_t); ++i) {
    // Configure
    if (timer0_write_byte(regABCArray[i], tmpArray[i])) {
      SC_REPORT_ERROR("Testbench::set_clock_enable()", "Can't write at T0@CH0@regA/B/C");
    }
    // Read data
    if (timer0_read_byte(regABCArray[i], &tmp)) {
      SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to read at T0@CH0@regA/B/C");
    }
    // Test value
    if (tmp != tmpArray[i]) {
      SC_REPORT_ERROR("Testbench::set_clock_enable()", "Unable to configure RA, RB and RC T0");
    }

  }



  printf("> TIMER CONFIGURATION: PASSED\n");
  return 0;
}


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
  printf("Timer interruption OK\n");

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
  printf("Channel interruption OK\n");

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

  set_write_protection(false);

  printf("> WRITE REGISTER A/B/C: PASSED\n");
  return 0;
}

int Testbench::test_counter_update(void)
{
  uint32_t tmp1, tmp2 = 0;

  printf("> BEGIN COUNTER UPDATE\n");

  // Set input clock
  if (timer0_read_byte(TC_CMR, &tmp1)) {
    SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to read at T0@CH0@CMR");
  }
  tmp1 = (tmp1 & ~(TC_CMRx_TCCLKS)) | 0; // 0: /2, 1: /8, 2: /32, 3: /128, 4: SLCK
  if (timer0_write_byte(TC_CMR, tmp1)) {
    SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to write at T0@CH0@CMR");
  }
  // if (timer0_write_byte(TC_SMMR, TC_SMMR_DOWN)) {
  //   SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to write at T0@CH0@SMMR");
  // }

  // Enable the clock by a counter reset
  if (timer0_write_byte(TC_CCR, TC_CCR_SWTRG)) {
    SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to write at T0@CH0@CCR");
  }

  if (timer0_read_byte(TC_CV, &tmp1)) {
    SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to read at T0@CH0@CV");
  }

  wait(1, SC_SEC);

  if (timer0_read_byte(TC_CV, &tmp2)) {
    SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to read at T0@CH0@CV");
  }

  printf("Before: %lu, After: %lu, Delta: %lu\n", tmp1, tmp2, tmp2 - tmp1);

  if (timer0_read_byte(TC_SR, &tmp1)) {
    SC_REPORT_ERROR("Testbench::test_counter_update()", "Unable to read at T0@CH0@SR");
  }

  if (tmp1 & TC_SR_COVFS) {
    printf("Overflow detected\n");
  }

  printf("> COUNTER UPDATE: PASSED\n");
  return 0;
}

/********************************************************
 *						TEST STEPS
 ********************************************************/

void Testbench::main_test(void) {
  set_pmc_data(0, 0);
  set_pmc_data(1 * KILO, 32 * KILO);
  test_timer_configuration();
  test_timer_address();
  test_write_protection();
  test_interruption();
  test_write_register_ABC();
  test_counter_update();
}
