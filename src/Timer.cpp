#include "Timer.h"
#include "Channel.h"
#include "Tools.h"

/**
 * Public methods
 */

Timer::Timer(sc_module_name name, uint32_t baseAddress) :
              sc_module(name),
              socket_PMC("socket_PMC"),
              socket_Bus("socket_Bus") {
  // Set internal variables
  this->baseAddress = baseAddress;
  this->curPmcData.mck = 0;                         // Timer starts disabled by default
  this->curPmcData.slck = 0;                        // Timer starts disabled by default
  memset(this->registerData, 0, sizeof(this->registerData));  // Reset value of internal registers
  this->isWriteProtected = false;                   // Write protection is disabled at reset

  // Instanciate channels
  channel1 = new Channel("Channel");

  // Link sockets
  socket_PMC.register_b_transport(this, &Timer::b_transport_pcm);
  socket_Bus.register_b_transport(this, &Timer::b_transport_bus);

}

void Timer::b_transport_pcm(tlm_generic_payload& trans, sc_time& delay)
{
    struct pmc_data *pmcData;

    // We support only write command on this socket
    if (trans.get_command() != TLM_WRITE_COMMAND) {
        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        return;
    }

    // The only data valid is a struct pmc_data, check length
    if (trans.get_data_length() != sizeof(struct pmc_data)) {
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        return;
    }

    // Cast and check the pointer
    pmcData = (struct pmc_data *) trans.get_data_ptr();
    if (pmcData == NULL) {
        trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
        return;
    }

    // Save data into internal attribute
    memcpy(&(this->curPmcData), pmcData, sizeof(struct pmc_data));

    // Print new frequency for debug
    cout << "Timer: new mck = " << this->curPmcData.mck << " Hz"
         << " new slck = " << this->curPmcData.slck << " Hz" << endl;

    trans.set_response_status(TLM_OK_RESPONSE);
}

void Timer::b_transport_bus(tlm_generic_payload& trans, sc_time& delay)
{
  uint32_t addr = trans.get_address();
  uint32_t *pData = (uint32_t *) trans.get_data_ptr();
  uint32_t cmd = trans.get_command();

  // Check if address is in our space
  if (!is_in_range(addr, this->baseAddress, TIMER_ADDR_SPACE)) {
    trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  // Check len: multiple register read/write not supported
  if (trans.get_data_length() != REGISTER_SIZE) {
    trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
    return;
  }

  // Remove addresse base offset
  addr -= this->baseAddress;

  if (is_in_range(addr, 0, TIMER_CHANNEL_ADDR_SPACE)) {
    // To channel 0
    if (channel1->manage_register(cmd, addr, pData) != 0) {
      trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
  } else if (is_in_range(addr, TIMER_CHANNEL_ADDR_SPACE, TIMER_CHANNEL_ADDR_SPACE)) {
    // To channel 1
    cout << "Received for Channel 1" << endl;
  } else if (is_in_range(addr, 2 * TIMER_CHANNEL_ADDR_SPACE, TIMER_CHANNEL_ADDR_SPACE)) {
    // To channel 2
    cout << "Received for Channel 2" << endl;
  } else {
    // To timer itself
    if (manage_register(cmd, addr, pData) != 0) {
      trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
  }
  trans.set_response_status(TLM_OK_RESPONSE);
}

/**
 * Private methods
 */

/**
 * @brief Manage read and write register for timer
 *
 * @param cmd TLM_WRITE_COMMAND or TLM_READ_COMMAND
 * @param address [TC_BCR to TC_WPMR]
 * @param pData Pointer on a uint32_t[1]
 * @return 0: Success, -1: Error
 */
int Timer::manage_register(uint8_t cmd, uint32_t address, uint32_t *pData)
{
  switch (address) {
    case TC_BCR:
      _is_write_only_();

      if ((*pData) & TC_BCR_SYNC) {
        // Do sync
      }
      break;

    case TC_BMR:
      if (_is_read()) {
        (*pData) = registerData[TC_BMR_I];
      } else {
        _need_wpen_();

      }
      break;

    case TC_QIER:
      _is_write_only_();

      if ((*pData) & TC_QIxR_Mask) {
        registerData[TC_QIMR_I] |= (*pData) & TC_QIxR_Mask;
      }
      break;

    case TC_QIDR:
      _is_write_only_();

      if ((*pData) & TC_QIxR_Mask) {
        registerData[TC_QIMR_I] &= ~((*pData) & TC_QIxR_Mask);
      }
      break;

    case TC_QIMR:
      _is_read_only_();
      // Just read enabled interrupts
      (*pData) = registerData[TC_QIMR_I] & TC_QIxR_Mask;
      break;

    case TC_QISR:
      _is_read_only_();
      // Mask all disabled interrupts
      (*pData) = registerData[TC_QISR_I] & registerData[TC_QIMR_I] & TC_QIxR_Mask;
      break;

    case TC_FMR:
      if (_is_read()) {
        (*pData) = registerData[TC_FMR_I];
      } else {
        _need_wpen_();
        this->registerData[TC_FMR_I] = (*pData);
      }
      break;

    case TC_WPMR:
      if (_is_read()) {
        (*pData) = registerData[TC_WPMR_I];
      } else {
        // Save only the WPEN bit
        this->registerData[TC_WPMR_I] = (*pData) & TC_WPMR_WPEN;

        // Is the password ok ?
        if (((*pData) >> 8) == TC_WPMR_PASSWORD) {
          if ((*pData) & TC_WPMR_WPEN) {
            set_write_protection(true);
          } else {
            set_write_protection(false);
          }
        }
      }
      break;

    default:
      fprintf(stderr, "Timer::manage_register() Ignoring %s at address: 0x%02X\n",
        _is_write() ? "write" : "read",
        address);
      return -1;
  }

  return 0;
}

/**
 * @brief Set the write protection on supported registers
 *
 * @param isEnabled boolean
 */
void Timer::set_write_protection(bool isEnabled)
{
  this->isWriteProtected = isEnabled;

  // Tell the channels
  channel1->set_write_protection(isEnabled);

  // Just debug
  cout << "Timer: Write protection is now ";
  if (isEnabled) {
    cout << "on" << endl;
  } else {
    cout << "off" << endl;
  }
}










