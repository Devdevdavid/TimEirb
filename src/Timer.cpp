#include "Timer.h"
#include "Channel.h"
#include "Tools.h"

Timer::Timer(sc_module_name name, uint32_t baseAddress) :
              sc_module(name),
              socket_PMC("socket_PMC"),
              socket_Bus("socket_Bus") {
  // Set internal variables
  this->baseAddress = baseAddress;
  this->curPmcData.mck = 0;                 // Timer starts disabled by default
  this->curPmcData.slck = 0;                // Timer starts disabled by default

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
  uint32_t data = *((uint32_t *) trans.get_data_ptr());
  uint32_t cmd = trans.get_command();

  // Check if address is in our space
  if (!is_in_range(addr, this->baseAddress, TIMER_ADDR_SPACE)) {
    trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }

  // Check len: multiple register read/write not supported
  if (trans.get_data_length() != 1) {
    trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
    return;
  }

  // Remove addresse base offset
  addr -= this->baseAddress;

  if (is_in_range(addr, 0, TIMER_CHANNEL_ADDR_SPACE)) {
    // To channel 0
    cout << "Received for Channel 0" << endl;
  } else if (is_in_range(addr, TIMER_CHANNEL_ADDR_SPACE, TIMER_CHANNEL_ADDR_SPACE)) {
    // To channel 1
    cout << "Received for Channel 1" << endl;
  } else if (is_in_range(addr, 2 * TIMER_CHANNEL_ADDR_SPACE, TIMER_CHANNEL_ADDR_SPACE)) {
    // To channel 2
    cout << "Received for Channel 2" << endl;
  } else {
    // To timer itself
    if (set_register(cmd, addr, data) != 0) {
      trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
  }
  trans.set_response_status(TLM_OK_RESPONSE);
}

int Timer::set_register(uint8_t cmd, uint32_t address, uint32_t value)
{
  switch (address) {
    case TC_BCR:
      _is_write_only_();

      if (value & TC_BCR_SYNC) {
        // Do sync
      }
      break;

    case TC_BMR:
      _is_read_write_();

      break;

    case TC_QIER:
      _is_write_only_();

      break;

    case TC_QIDR:
      _is_write_only_();

      break;

    case TC_QIMR:
      _is_read_only_();

      break;

    case TC_QISR:
      _is_read_only_();

      break;

    case TC_FMR:
      _is_read_write_();

      break;

    case TC_WPMR:
      _is_read_write_();

      break;

    default:
      fprintf(stderr, "Timer::set_register() Ignoring %s at address: 0x%02X\n",
        _is_write() ? "write" : "read",
        address);
      return -1;
  }

  return 0;
}











