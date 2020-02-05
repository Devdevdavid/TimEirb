#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "tlm_head.h"
#include <iostream>

SC_MODULE(Channel) {
  SC_CTOR(Channel);

  void setMck(uint32_t mclkFreq, uint32_t sclkFreq);
  void setReg(uint8_t addr, uint32_t value, uint8_t length);
  void getReg(uint8_t addr, uint32_t value, uint8_t length);
  void initInterrupt(void *interruptMethod);
  void getTioa(struct tio tioa);
  void getTiob(struct tio tiob);
  void setSync();
};

struct tio {
  uint32_t frequency;
  uint16_t duty_cycle;
};

#endif /* _CHANNEL_H_ */
