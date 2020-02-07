#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <iostream>

#include "ChannelRegister.h"
#include "tlm_head.h"

// ADDRESSES
#define TIMER_CHANNEL_ADDR_SPACE  0x40

struct clockFrequency {
  uint32_t mclk;
  uint32_t sclk;
  uint32_t clock_counter;
  uint32_t generated_clock;
};

struct tio {
  uint32_t frequency;
  uint16_t duty_cycle;
};

SC_MODULE(Channel) {
  /*
   * public methods
   */
public:
  SC_CTOR(Channel);

  void setInputClock(uint32_t mclkFreq, uint32_t sclkFreq);
  void setReg(uint8_t addr, uint32_t value, uint8_t length);
  void getReg(uint8_t addr, uint32_t value, uint8_t length);
  void initInterrupt(void *interruptMethod);
  void getTioa(struct tio tioa);
  void getTiob(struct tio tiob);
  void setSync(void);

  /*
   * private members
   */
private:
  uint32_t register_data[16];
  struct clockFrequency clk;

  /*
   * private methods
   */
private:
  bool clockEnable(void);
  void UpdateClockCounter(void);
  void tioUpdate(void);
  void generatedClockUpdate(void);
};

#endif /* _CHANNEL_H_ */
