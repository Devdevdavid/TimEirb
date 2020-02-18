#include "Channel.h"

/**
 * Public methods
 */

Channel::Channel(sc_module_name name) : sc_module(name), tioSocket("TioSocket")
{
    memset(this->registerData, 0, sizeof(registerData));  // Reset value of internal registers
    memset(&this->curPmcData, 0, sizeof(struct pmc_data));// Reset value of PMC data
    memset(&this->curTioData, 0, sizeof(struct socket_tio_data_t));// Reset value of TIO data

    this->channelId = 0;                                  // The timer gives the ID with set_channel_id()
    this->counterClockFreqHz = 0;                         // Clock is disabled at reset
    this->isWriteProtected = false;                       // Write protection is disabled at reset
    this->lastCounterUpdate = SC_ZERO_TIME;               // Last update is at 0 sec
    this->mInterruptMethod = NULL;                        // No interrupt method yet

    //interrupt
    SC_THREAD(counter_overflow);
    SC_THREAD(load_overrun);
    SC_THREAD(RA_compare);
    SC_THREAD(RB_compare);
    SC_THREAD(RC_compare);
    SC_THREAD(RA_loading);
    SC_THREAD(RB_loading);
}

/**
 * @brief Manage read and write register for channel
 *
 * @param cmd TLM_WRITE_COMMAND or TLM_READ_COMMAND
 * @param address [TC_CCR to TC_IMR]
 * @param pData Pointer on a uint32_t[1]
 * @return 0: Success, -1: Error
 */
int Channel::manage_register(uint8_t cmd, uint32_t address, uint32_t *pData)
{
  switch (address) {
    case TC_CCR:              /** Control */
      _is_write_only_();

      // Test bits but no need to save values
      // Reset timer
      if ((*pData) & TC_CCR_SWTRG) {
        reset_counter();
      }
      else {
        // Stop the timer clock
        if ((*pData) & TC_CCR_CLKDIS) {
          set_clock_enable(false);
        } else {
          // Start the timer clock
          if ((*pData) & TC_CCR_CLKEN) {
            set_clock_enable(true);
          }
        }
      }
      break;

    case TC_CMR:              /** Mode */
      if (_is_read()) {
        (*pData) = registerData[TC_CMR_I];
      } else {
        _need_wpen_();
        registerData[TC_CMR_I] = (*pData) & TC_CMRx_Mask;
        // TCCLKS changed
      update_counter_clock(registerData[TC_CMR_I] & TC_CMRx_TCCLKS);
        // TC_CMRw_CPCSTOP changed
        if((*pData) & TC_CMRw_CPCSTOP){
          set_clock_enable(false);
        }
        // TC_CMRw_CPCDIS changed
        if((*pData) & TC_CMRw_CPCDIS){
          set_clock_enable(false);
        }
        // TC_CMRw_EEVTEDG changed
        if((*pData) & TC_CMRw_ENETRG){
          reset_counter();
          set_clock_enable(true);
        }
        // TC_CMRw_WAVSEL changed
        if((*pData) & TC_CMRw_WAVSEL){
          //update_interrupt_thread();
          waveformSelection = ((*pData) & TC_CMRw_WAVSEL);
        }
        // TC_CMRw_WAVE changed
        if ((*pData) & TC_CMRx_WAVE == 0) {
          return -1;
        }
      }
      break;

    case TC_SMMR:             /** Mode Motor */
      if (_is_read()) {
        (*pData) = registerData[TC_SMMR_I];
      } else {
        _need_wpen_();
        registerData[TC_SMMR_I] = (*pData) & TC_SMMR_Mask;
        if((*pData) & TC_SMMR_GCEN) {
          set_clock_enable(true);
        }
      }
      break;

    case TC_CV:               /** Value */
      _is_read_only_();

      update_counter_value();
      (*pData) = registerData[TC_CV_I];
      break;

    case TC_RA:               /** Register A */
      if (_is_read()) {
        (*pData) = registerData[TC_RA_I];
      } else {
        _need_wpen_();

        // Check wave mode before write this register
        if ((registerData[TC_CMR_I] & TC_CMRx_WAVE) == 0) {
          return -1;
        }

        registerData[TC_RA_I] = (*pData);
      }
      break;

    case TC_RB:               /** Register B */
      if (_is_read()) {
        (*pData) = registerData[TC_RB_I];
      } else {
        _need_wpen_();

        // Check wave mode before write this register
        if ((registerData[TC_CMR_I] & TC_CMRx_WAVE) == 0) {
          return -1;
        }

        registerData[TC_RB_I] = (*pData);
      }
      break;

    case TC_RC:               /** Register C */
      if (_is_read()) {
        (*pData) = registerData[TC_RC_I];
      } else {
        _need_wpen_();
        registerData[TC_RC_I] = (*pData);
      }
      break;

    case TC_SR:               /** Status */
      _is_read_only_();

      // Read TC_SR_COVFS
      (*pData) = registerData[TC_SR_I] & (TC_SR_COVFS);

      // Read TC_SR_LOVRS
      (*pData) = registerData[TC_SR_I] & (TC_SR_LOVRS);

      // Read TC_SR_LDRAS
      (*pData) = registerData[TC_SR_I] & (TC_SR_LDRAS);

      // Read TC_SR_LDRBS
      (*pData) = registerData[TC_SR_I] & (TC_SR_LDRBS);

      // Read TC_SR_ETRGS
      (*pData) = registerData[TC_SR_I] & (TC_SR_ETRGS);

      // Read other status
      (*pData) = registerData[TC_SR_I] & (TC_SR_MTIOB | TC_SR_MTIOA | TC_SR_CLKSTA);

      // Read interrupts
      (*pData) |= registerData[TC_SR_I] & registerData[TC_IMR_I] & TC_IxR_Mask;

      // Clear interrupts status on read
      registerData[TC_IMR_I] &= ~(TC_IxR_Mask);
      break;

    case TC_IER:               /** Enable interrupts */
      _is_write_only_();

      // Enable bits only if they are in the mask
      registerData[TC_IMR_I] |= (*pData) & TC_IxR_Mask;

      //update Interrupt enable
      update_interrupt_thread();
      break;

    case TC_IDR:               /** Disable interrupts */
      _is_write_only_();

      // Disable bits only if they are in the mask
      registerData[TC_IMR_I] &= ~((*pData) & TC_IxR_Mask);

      //update Interupt event
      update_interrupt_thread();
      break;

    case TC_IMR:               /** Interrupt mask */
      _is_read_only_();

      // Just read enabled interrupts
      (*pData) = registerData[TC_IMR_I] & TC_IxR_Mask;
      break;

    default:
      fprintf(stderr, "Channel::manage_register() Ignoring %s at address: 0x%02X\n",
        _is_write() ? "write" : "read",
        address);
      return -1;
  }

  return 0;
}

void Channel::set_channel_id(uint8_t channelId)
{
  this->channelId = channelId;
}

void Channel::set_write_protection(bool isEnabled)
{
  this->isWriteProtected = isEnabled;
}

void Channel::set_pmc_clock(const struct pmc_data &pmcData)
{
  // Update counter value before updating clocks
  update_counter_value();

  // Copy data into internal variables
  memcpy(&(this->curPmcData), &pmcData, sizeof(struct pmc_data));

  // Update the divided clock value
  update_counter_clock(registerData[TC_CMR_I] & TC_CMRx_TCCLKS);
}

/*
* private methods
*/
void Channel::update_counter_clock(uint8_t TCCLKSValue)
{
  switch (TCCLKSValue) {
    case 0:
      this->counterClockFreqHz = this->curPmcData.mck / 2;
      break;

    case 1:
      this->counterClockFreqHz = this->curPmcData.mck / 8;
      break;

    case 2:
      this->counterClockFreqHz = this->curPmcData.mck / 32;
      break;

    case 3:
      this->counterClockFreqHz = this->curPmcData.mck / 128;
      break;

    case 4:
      this->counterClockFreqHz = this->curPmcData.slck;
      break;

    default:
      this->counterClockFreqHz = 0; // Turned off
      fprintf(stderr, "Channel::update_counter_clock() \
        Ignoring clock choice (XC0, XC1 and XC2 not supported)\n");
      break;
  }
}

void Channel::update_counter_value(void)
{
  sc_time delta;
  int64_t deltaCount;
  int64_t counterValue;

  // Ignore updates if counter clock is not enabled
  if ((registerData[TC_SR_I] & TC_SR_CLKSTA) == 0) {
    return;
  }

  // Compute the delay since the last counter update
  delta = sc_time_stamp() - lastCounterUpdate;

  // Compute delta value in count cycle
  deltaCount = floor(this->counterClockFreqHz * delta.to_seconds());

  // Apply
  counterValue = registerData[TC_CV_I];
  if (registerData[TC_SMMR_I] & TC_SMMR_DOWN) {
    counterValue -= deltaCount;
  } else {
    counterValue += deltaCount;
  }

  // Save new value
  registerData[TC_CV_I] = counterValue;

  // Update the timestamp
  lastCounterUpdate = sc_time_stamp();
}

void Channel::tio_update(void)
{
  int ret = 0;
  sc_time delay = sc_time(0, SC_NS);
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_address(this->channelId);        // Identify the channel for the testbench
  trans->set_data_ptr((uint8_t *) &curTioData);
  trans->set_data_length(sizeof(struct socket_tio_data_t));
  trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

  // Send it !
  this->tioSocket->b_transport(*trans, delay);

  // Print error condition
  if (trans->is_response_error()) {
    char rwChar = (trans->get_command() == TLM_READ_COMMAND) ? 'R' : 'W';
    //fprintf(stderr, "Testbench::socket_action() %c@0x%04X L=%dB\n", rwChar, trans->get_address(), trans->get_data_length());
  }

  if (trans->is_response_error()) {
    fprintf(stderr, "Channel::tio_update() Transaction failed: %s\n", trans->get_response_string().c_str());
  }

  delete (trans);
}

void Channel::get_tioa(struct tio_t tioa)
{
}

void Channel::get_tiob(struct tio_t tiob)
{
}

void Channel::reset_counter(void)
{
  set_clock_enable(true);

  // Force timestamp then reset
  lastCounterUpdate = sc_time_stamp();
  registerData[TC_CV_I] = 0;
}

void Channel::set_clock_enable(bool isEnabled)
{
  if (isEnabled) {
    // INACTIVE to ACTIVE: set timestamp to now
    lastCounterUpdate = sc_time_stamp();
    // Save
    registerData[TC_SR_I] |= TC_SR_CLKSTA;
  } else {
    // ACTIVE to INACTIVE: update counter now
    update_counter_value();
    // Save
    registerData[TC_SR_I] &= ~TC_SR_CLKSTA;
  }
}

 void Channel::init_interrupt(void *interruptMethod)
 {
     mInterruptMethod = interruptMethod;
 }

 void Channel::update_interrupt_thread()
 {
 }

  void Channel::counter_overflow() {}
  void Channel::load_overrun() {}
  void Channel::RA_compare() {}
  void Channel::RB_compare() {}
  void Channel::RC_compare() {}
  void Channel::RA_loading() {}
  void Channel::RB_loading() {}
