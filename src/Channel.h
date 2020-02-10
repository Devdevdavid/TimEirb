#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <iostream>

#include "tlm_head.h"
#include "Tools.h"

// ADDRESSES
#define TIMER_CHANNEL_ADDR_SPACE  0x40

// RESGISTERS ADDRESS TO TIMER ITSELF
#define TC_CCR        0x00
#define TC_CMR        0x04
#define TC_SMMR       0x08
#define TC_CV         0x10
#define TC_RA         0x14
#define TC_RB         0x18
#define TC_RC         0x1C
#define TC_SR         0x20
#define TC_IER        0x24
#define TC_IDR        0x28
#define TC_IMR        0x2C

// REGISTER INDEXES
#define TC_CCR_I      0
#define TC_CMR_I      1
#define TC_SMMR_I     2
#define TC_CV_I       3
#define TC_RA_I       4
#define TC_RB_I       5
#define TC_RC_I       6
#define TC_SR_I       7
#define TC_IER_I      8
#define TC_IDR_I      9
#define TC_IMR_I      10
#define TCC_REG_COUNT 11

/** TC_CCR bit definition */
#define TC_CCR_CLKEN   (1 << 0)
#define TC_CCR_CLKDIS  (1 << 1)
#define TC_CCR_SWTRG   (1 << 2)

/** TC_CMR bit definition */
#define TC_CMR_TCCLKS  (0x07 << 0)
#define TC_CMR_CPCSTOP (1 << 6)
#define TC_CMR_CPCDIS  (1 << 7)

/** TC_SMMR bit definition */

/** TC_CV bit definition */

/** TC_RA bit definition */

/** TC_RB bit definition */

/** TC_RC bit definition */

/** TC_SR bit definition */

/** TC_IER bit definition */

/** TC_IDR bit definition */

/** TC_IMR bit definition */


struct clockFrequency {
  uint32_t mclk;
  uint32_t sclk;
  uint32_t clock_counter;
  uint32_t generated_clock;
};

struct tio {
  uint32_t clock_frequency;
  uint16_t duty_cycle;
};

SC_MODULE(Channel) {
  /*
   * public methods
   */
public:
  SC_CTOR(Channel);
  int manage_register(uint8_t cmd, uint32_t address, uint32_t *pData);
  void set_write_protection(bool isEnabled);

  void setInputClock(uint32_t mclkFreq, uint32_t sclkFreq);
  void setReg(uint8_t addr, uint32_t value, uint8_t length);
  void getReg(uint8_t addr, uint32_t value, uint8_t length);
  void initInterrupt(void *interruptMethod);
  void getTioa(struct tio tioa);
  void getTiob(struct tio tiob);
  void setSync(void);

  /*
   * private methods
   */
private:
  bool clockEnable(void);
  void UpdateClockCounter(void);
  void tioUpdate(void);
  void generatedClockUpdate(void);

  /*
   * private members
   */
private:
  uint32_t registerData[TCC_REG_COUNT];
  struct clockFrequency clk;
  bool isWriteProtected;                /** Tell if Write protection is enabled (Works on some registers) */
};

#endif /* _CHANNEL_H_ */
