#include "Channel.h"

/**
 * Public methods
 */

Channel::Channel(sc_module_name name) : sc_module(name)
{
    this->isWriteProtected = false;                   // Write protection is disabled at reset
    memset(this->registerData, 0, sizeof(registerData)); // Reset value of internal registers
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
    case TC_CCR:
      _is_write_only_();
      break;

    case TC_CMR:
      if (_is_read()) {

      } else {
        _need_wpen_();

      }
      break;

    case TC_SMMR:
      if (_is_read()) {

      } else {
        _need_wpen_();

      }

      break;

    case TC_CV:
      _is_read_only_();

      break;

    case TC_RA:
      if (_is_read()) {

      } else {
        _need_wpen_();
      }
      break;

    case TC_RB:
      if (_is_read()) {

      } else {
        _need_wpen_();
      }
      break;

    case TC_RC:
      if (_is_read()) {

      } else {
        _need_wpen_();
      }
      break;

    case TC_SR:
      _is_read_only_()
      break;

    case TC_IER:
      _is_write_only_();
      break;

    case TC_IDR:
      _is_write_only_();
      break;

    case TC_IMR:
      _is_read_only_();
      break;

    default:
      fprintf(stderr, "Channel::manage_register() Ignoring %s at address: 0x%02X\n",
        _is_write() ? "write" : "read",
        address);
      return -1;
  }

  return 0;
}

void Channel::set_write_protection(bool isEnabled)
{
  this->isWriteProtected = isEnabled;
}

/*
* private methods
*/

void Channel::generatedClockUpdate(void)
{
    switch (registerData[TC_CMR] & TC_CMR_TCCLKS) {
        case 0 :
            clk.generated_clock = clk.mclk / 2;
        break;

        case 1 :
            clk.generated_clock = clk.mclk / 8;
        break;

        case 2 :
            clk.generated_clock = clk.mclk / 32;
        break;

        case 3 :
            clk.generated_clock = clk.mclk / 128;
        break;

        case 4 :
            clk.generated_clock = clk.sclk;
        break;
    }
}

void Channel::UpdateClockCounter(void)
{
    if (clockEnable())
        clk.clock_counter = clk.generated_clock;
}

bool Channel::clockEnable(void)
{
    if (((registerData[TC_CCR_I] & TC_CCR_CLKDIS) == (1 << 1))
     || ((registerData[TC_CMR_I] & TC_CMR_CPCSTOP) == (1 << 6))
     || ((registerData[TC_CCR_I] & TC_CCR_CLKDIS) == (1 << 7)))
    {
        return false;
    }
    else if ((registerData[TC_CCR_I] & TC_CCR_CLKEN) == 1)
    {
        return true;
    }
    return true;
}

void Channel::tioUpdate(void)
{
}

void Channel::getTioa(struct tio tioa)
{
}