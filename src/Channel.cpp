#include <inttypes.h>
#include "Channel.h"

/**
 * Public methods
 */

Channel::Channel(sc_module_name name) : sc_module(name), tioSocket("TioSocket"), intSocket("IntSocket")
{
    memset(this->registerData, 0, sizeof(registerData));  // Reset value of internal registers
    memset(&this->curPmcData, 0, sizeof(struct pmc_data));// Reset value of PMC data
    memset(&this->curTioData, 0, sizeof(struct socket_tio_data_t));// Reset value of TIO data

    this->channelId = 0;                                  // The timer gives the ID with set_channel_id()
    this->counterClockFreqHz = 0;                         // Clock is disabled at reset
    this->isWriteProtected = false;                       // Write protection is disabled at reset
    this->lastCounterUpdate = SC_ZERO_TIME;               // Last update is at 0 sec

    SC_METHOD(method_update_next_event);
    sensitive << nextUpdateEvent;
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
        // Clock changed, update events
        method_update_next_event();
      }
      break;

    case TC_CMR:              /** Mode */
      if (_is_read()) {
        (*pData) = registerData[TC_CMR_I];
      } else {
        _need_wpen_();
        registerData[TC_CMR_I] = (*pData) & TC_CMRx_Mask;

        // TCCLKS changed
        update_counter_clock();

        // Save the new waveform mode
        waveformSelection = (registerData[TC_CMR_I] & TC_CMRw_WAVSEL) >> 13;
        isInWaveformMode = registerData[TC_CMR_I] & TC_CMRx_WAVE;
      }
      break;

    case TC_SMMR:             /** Mode Motor */
      if (_is_read()) {
        (*pData) = registerData[TC_SMMR_I];
      } else {
        _need_wpen_();
        registerData[TC_SMMR_I] = (*pData) & TC_SMMR_Mask;
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
        if (!isInWaveformMode) {
          return -1;
        }

        registerData[TC_RA_I] = (*pData);

        // CompA changed, update event
        method_update_next_event();
      }
      break;

    case TC_RB:               /** Register B */
      if (_is_read()) {
        (*pData) = registerData[TC_RB_I];
      } else {
        _need_wpen_();

        // Check wave mode before write this register
        if (!isInWaveformMode) {
          return -1;
        }

        registerData[TC_RB_I] = (*pData);

        // CompB changed, update event
        method_update_next_event();
      }
      break;

    case TC_RC:               /** Register C */
      if (_is_read()) {
        (*pData) = registerData[TC_RC_I];
      } else {
        _need_wpen_();
        registerData[TC_RC_I] = (*pData);

        // CompC changed, update event
        method_update_next_event();
      }
      break;

    case TC_SR:               /** Status */
      _is_read_only_();

      // Read interrupts
      (*pData) = registerData[TC_SR_I] & registerData[TC_IMR_I] & TC_IxR_Mask;

      // Clear interrupts status on read
      registerData[TC_IMR_I] &= ~(TC_IxR_Mask);
      break;

    case TC_IER:               /** Enable interrupts */
      _is_write_only_();

      // Enable bits only if they are in the mask
      registerData[TC_IMR_I] |= (*pData) & TC_IxR_Mask;
      break;

    case TC_IDR:               /** Disable interrupts */
      _is_write_only_();

      // Disable bits only if they are in the mask
      registerData[TC_IMR_I] &= ~((*pData) & TC_IxR_Mask);
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
  update_counter_clock();
}

/*
* private methods
*/

#define NB_DELTA  4
void Channel::method_update_next_event(void)
{
  sc_time deltaTime;
  int64_t deltaCount;
  int64_t deltaList[NB_DELTA];

  // Stop any running timeout
  nextUpdateEvent.cancel();

  // Clock disabled ?
  if ((registerData[TC_SR_I] & TC_SR_CLKSTA) == 0) {
    return;
  }
  if (this->counterClockFreqHz == 0) {
    return;
  }

  // Do stuff to update values
  update_counter_value();
  tio_update();

  // Compute next event
  deltaList[0] = (int64_t) registerData[TC_RA_I] - registerData[TC_CV_I];
  deltaList[1] = (int64_t) registerData[TC_RB_I] - registerData[TC_CV_I];
  deltaList[2] = (int64_t) registerData[TC_RC_I] - registerData[TC_CV_I];
  deltaList[3] = (int64_t) UINT32_MAX - registerData[TC_CV_I];

  // Take the opposite if counting down
  for (int i = 0; i < NB_DELTA; ++i) {
    //printf("delta[%d] = %jd\n", i, deltaList[i]);
    if (registerData[TC_SMMR_I] & TC_SMMR_DOWN) {
      deltaList[i] = -deltaList[i];
    }
  }

  // Get the value the most close to 0 still positive
  deltaCount = get_min_pos_value(deltaList, NB_DELTA);
  deltaTime = sc_time((deltaCount * GIGA) / this->counterClockFreqHz, SC_NS);
  // Wait
  //cout << "Waiting for " << to_string((uint32_t) (deltaTime.to_seconds() * GIGA)) << "ns (deltaCount = " << to_string(deltaCount) << ")" << endl;
  nextUpdateEvent.notify(deltaTime);
}

void Channel::update_counter_clock()
{
  switch (registerData[TC_CMR_I] & TC_CMRx_TCCLKS) {
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

  // Clock changed, timeout need update
  method_update_next_event();
}

void Channel::update_counter_value(void)
{
  sc_time delta;
  int64_t deltaCount;
  int64_t counterValue;

  // Ignore updates if counter clock is not enabled or null
  if ((registerData[TC_SR_I] & TC_SR_CLKSTA) == 0) {
    return;
  }
  if (this->counterClockFreqHz == 0) {
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

  if ((counterValue < 0) || (counterValue > UINT32_MAX)) {
    // Modulo
    while (counterValue < 0) { counterValue += UINT32_MAX; }
    while (counterValue > UINT32_MAX) { counterValue -= UINT32_MAX; }

    // Trigger Overflow interrupt
    if (registerData[TC_IMR_I] & TC_IxR_COVFS) {
      registerData[TC_SR_I] |= TC_SR_COVFS;
      send_int_update(TC_IxR_COVFS);
    }
  }

  // Save new value
  registerData[TC_CV_I] = counterValue;

  if (registerData[TC_IMR_I] & TC_IxR_CPAS) {
    if (registerData[TC_CV_I] == registerData[TC_RA_I]) {
      registerData[TC_SR_I] |= TC_SR_CPAS;
      send_int_update(TC_IxR_CPAS);
    }
  }
  if (registerData[TC_IMR_I] & TC_IxR_CPBS) {
    if (registerData[TC_CV_I] == registerData[TC_RB_I]) {
      registerData[TC_SR_I] |= TC_SR_CPBS;
      send_int_update(TC_IxR_CPBS);
    }
  }
  if (registerData[TC_IMR_I] & TC_IxR_CPCS) {
    if (registerData[TC_CV_I] == registerData[TC_RC_I]) {
      registerData[TC_SR_I] |= TC_SR_CPCS;
      send_int_update(TC_IxR_CPCS);
    }
  }
  //printf("CV = %d\n", registerData[TC_CV_I]);

  // Update the timestamp
  lastCounterUpdate = sc_time_stamp();
}

void Channel::tio_update(void)
{
  struct socket_tio_data_t tmpTioData;
  memset(&tmpTioData, 0, sizeof(struct socket_tio_data_t));


  if (isInWaveformMode && (registerData[TC_SR_I] & TC_SR_CLKSTA) && (this->counterClockFreqHz != 0)) {
    switch (waveformSelection) {
      case 0:
        tmpTioData.tioA.clockFrequency = this->counterClockFreqHz / UINT32_MAX;
        tmpTioData.tioA.dutyCycle = ((registerData[TC_RC_I] - registerData[TC_RB_I]) * 100) / UINT32_MAX;
        tmpTioData.tioB.clockFrequency = this->counterClockFreqHz / UINT32_MAX;
        tmpTioData.tioB.dutyCycle = ((registerData[TC_RC_I] - registerData[TC_RA_I]) * 100) / UINT32_MAX;
        break;

      case 2:
        // Do not divide by 0
        if (registerData[TC_RC_I] == 0) {
          break;
        }
        // Check order
        if (registerData[TC_RA_I] > registerData[TC_RB_I]) {
          break;
        }
        if (registerData[TC_RB_I] > registerData[TC_RC_I]) {
          break;
        }
        tmpTioData.tioA.clockFrequency = this->counterClockFreqHz / registerData[TC_RC_I];
        tmpTioData.tioA.dutyCycle = ((registerData[TC_RC_I] - registerData[TC_RB_I]) * 10 * KILO) / registerData[TC_RC_I];
        tmpTioData.tioB.clockFrequency = this->counterClockFreqHz / registerData[TC_RC_I];
        tmpTioData.tioB.dutyCycle = ((registerData[TC_RC_I] - registerData[TC_RA_I]) * 10 * KILO) / registerData[TC_RC_I];
        break;

      default:
        fprintf(stderr, "Waveform Mode %d is not supported\n", waveformSelection);
        break;
    }
  }

  // Save and send if it changed
  if (memcmp(&(this->curTioData), &tmpTioData, sizeof(struct socket_tio_data_t)) != 0) {
    memcpy(&(this->curTioData), &tmpTioData, sizeof(struct socket_tio_data_t));
    send_tio_update();
  }
}

void Channel::send_tio_update(void)
{
  int ret = 0;
  sc_time delay = SC_ZERO_TIME;
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_address(this->channelId);        // Identify the channel for the testbench
  trans->set_data_ptr((uint8_t *) &curTioData);
  trans->set_data_length(sizeof(struct socket_tio_data_t));
  trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

  // Send it !
  this->tioSocket->b_transport(*trans, delay);

  if (trans->is_response_error()) {
    fprintf(stderr, "Channel::tio_update() Transaction failed: %s\n", trans->get_response_string().c_str());
  }

  delete (trans);
}

void Channel::send_int_update(uint32_t intId)
{
  sc_time delay = SC_ZERO_TIME;
  tlm_generic_payload *trans = new tlm_generic_payload;

  trans->set_address(this->channelId);        // Identify the channel for the testbench
  trans->set_data_ptr((uint8_t *) &intId);
  trans->set_data_length(sizeof(uint32_t));
  trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

  // Send it !
  this->intSocket->b_transport(*trans, delay);

  if (trans->is_response_error()) {
    fprintf(stderr, "Channel::send_int_update() Transaction failed: %s\n", trans->get_response_string().c_str());
  }

  delete (trans);
}

void Channel::reset_counter(void)
{
  set_clock_enable(true);

  // Force timestamp then reset
  lastCounterUpdate = sc_time_stamp();
  registerData[TC_CV_I] = 0;

  // Clock changed, update events
  method_update_next_event();
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
