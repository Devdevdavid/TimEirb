#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <iostream>

#include "tlm_head.h"
#include "pmc_interface.h"
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
// Common
#define TC_CMRx_TCCLKS  (7 << 0)
#define TC_CMRx_CLKI    (1 << 3)
#define TC_CMRx_BURST   (3 << 4)
#define TC_CMRx_WAVE    (1 << 15)
#define TC_CMRx_Mask    (TC_CMRc_Mask | TC_CMRw_Mask)

// For capture mode
#define TC_CMRc_LDBSTOP (1 << 6)
#define TC_CMRc_LDBDIS  (1 << 7)
#define TC_CMRc_ETRGEDG (3 << 8)
#define TC_CMRc_ABETRG  (1 << 10)
#define TC_CMRc_CPCTRG  (1 << 14)
#define TC_CMRc_LDRA    (3 << 16)
#define TC_CMRc_LDRB    (3 << 18)
#define TC_CMRc_Mask    (0x000FC7FF)

// For waveform mode
#define TC_CMRw_CPCSTOP (1 << 6)
#define TC_CMRw_CPCDIS  (1 << 7)
#define TC_CMRw_EEVTEDG (3 << 8)
#define TC_CMRw_EEVT    (3 << 10)
#define TC_CMRw_ENETRG  (1 << 11)
#define TC_CMRw_WAVSEL  (3 << 13)
#define TC_CMRw_ACPA    (3 << 16)
#define TC_CMRw_ACPC    (3 << 18)
#define TC_CMRw_AEEVT   (3 << 20)
#define TC_CMRw_ASWTRG  (3 << 22)
#define TC_CMRw_BCPB    (3 << 24)
#define TC_CMRw_BCPC    (3 << 26)
#define TC_CMRw_BEEVT   (3 << 28)
#define TC_CMRw_BSWTRG  (3 << 30)
#define TC_CMRw_Mask    (0xFFFFFFFF)

/** TC_SMMR bit definition */
#define TC_SMMR_GCEN   (1 << 0)
#define TC_SMMR_DOWN   (1 << 1)
#define TC_SMMR_Mask   (TC_SMMR_DOWN | TC_SMMR_GCEN)

/** TC_CV bit definition */
#define TC_CV_Mask     (0xFFFFFFFF)

/** TC_RA bit definition */
#define TC_RA_Mask     (0xFFFFFFFF)

/** TC_RB bit definition */
#define TC_RB_Mask     (0xFFFFFFFF)

/** TC_RC bit definition */
#define TC_RC_Mask     (0xFFFFFFFF)

/** TC_SR bit definition */
#define TC_SR_COVFS    (1 << 0)
#define TC_SR_LOVRS    (1 << 1)
#define TC_SR_CPAS     (1 << 2)
#define TC_SR_CPBS     (1 << 3)
#define TC_SR_CPCS     (1 << 4)
#define TC_SR_LDRAS    (1 << 5)
#define TC_SR_LDRBS    (1 << 6)
#define TC_SR_ETRGS    (1 << 7)
#define TC_SR_CLKSTA   (1 << 16)
#define TC_SR_MTIOA    (1 << 17)
#define TC_SR_MTIOB    (1 << 18)
#define TC_SR_Mask     (TC_SR_MTIOB | TC_SR_MTIOA | TC_SR_CLKSTA | 0x000700FF)

/** TC_IER bit definition */
/** TC_IDR bit definition */
/** TC_IMR bit definition */
#define TC_IxR_COVFS    (1 << 0)
#define TC_IxR_LOVRS    (1 << 1)
#define TC_IxR_CPAS     (1 << 2)
#define TC_IxR_CPBS     (1 << 3)
#define TC_IxR_CPCS     (1 << 4)
#define TC_IxR_LDRAS    (1 << 5)
#define TC_IxR_LDRBS    (1 << 6)
#define TC_IxR_ETRGS    (1 << 7)
#define TC_IxR_Mask     (0x000000FF)

struct tio_t {
  uint32_t clockFrequency;
  uint16_t dutyCycle;
};

SC_MODULE(Channel) {
  /*
   * public methods
   */
public:
  SC_CTOR(Channel);
  int manage_register(uint8_t cmd, uint32_t address, uint32_t *pData);
  void set_write_protection(bool isEnabled);
  void set_pmc_clock(const struct pmc_data &pmcData);

  void init_interrupt(void *interruptMethod);
  void get_tioa(struct tio_t tioa);
  void get_tiob(struct tio_t tiob);
  void set_sync(void);

  /*
   * private methods
   */
private:
  void update_counter_value(void);
  void tio_update(void);
  void update_counter_clock(void);
  void reset_counter(void);
  void set_clock_enable(bool isEnabled);

  /*
   * private members
   */
private:
  uint32_t registerData[TCC_REG_COUNT];
  uint32_t counterClockFreqHz;          /** The value in hertz of the divided channel clock (0 means turned off) */
  struct pmc_data curPmcData;           /** Local copy of pmcData given by the Timer */
  bool isWriteProtected;                /** Tell if Write protection is enabled (Works on some registers) */
  sc_time lastCounterUpdate;            /** Indicates the last simulation instant when the counter had been updated */
};

#endif /* _CHANNEL_H_ */
