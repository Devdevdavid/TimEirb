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

    case TC_CMR:
      if (_is_read()) {
        (*pData) = registerData[TC_CMR_I];
      } else {
        _need_wpen_();
        registerData[TC_CMR_I] = (*pData) & TC_CMRx_Mask;
      }
      break;

    case TC_SMMR:
      if (_is_read()) {
        (*pData) = registerData[TC_SMMR_I];
      } else {
        _need_wpen_();
        registerData[TC_SMMR_I] = (*pData) & TC_SMMR_Mask;
      }

      break;

    case TC_CV:
      _is_read_only_();
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

void Channel::set_write_protection(bool isEnabled)
{
  this->isWriteProtected = isEnabled;
}

/*
* private methods
*/
void Channel::generated_clock_update(void)
{
  switch (registerData[TC_CMR_I] & TC_CMRx_TCCLKS) {
    case 0:
      clk.generatedClock = clk.mclk / 2;
      break;

    case 1:
      clk.generatedClock = clk.mclk / 8;
      break;

    case 2:
      clk.generatedClock = clk.mclk / 32;
      break;

    case 3:
      clk.generatedClock = clk.mclk / 128;
      break;

    case 4:
      clk.generatedClock = clk.sclk;
      break;

    default:
      clk.generatedClock = 0;
      fprintf(stderr, "Channel::generated_clock_update() \
        Ignoring clock choice (XC0, XC1 and XC2 not supported)\n");
      break;
  }
}

void Channel::update_clock_counter(void)
{
  if (clock_enable()) {
    clk.clockCounter = clk.generatedClock;
  }
}

bool Channel::clock_enable(void)
{
  if (((registerData[TC_CCR_I] & TC_CCR_CLKDIS) == (1 << 1))
    || ((registerData[TC_CMR_I] & TC_CMRw_CPCSTOP) == (1 << 6))
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

void Channel::tio_update(void)
{
}

void Channel::get_tioa(struct tio_t tioa)
{
}

void Channel::reset_counter(void)
{
  set_clock_enable(true);
}

void Channel::set_clock_enable(bool isEnabled)
{

}