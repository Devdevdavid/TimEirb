#ifndef _PMC_INTERFACE_H_
#define _PMC_INTERFACE_H_

#include <cstdint>

/**
 * @brief Data Structure used by PMC Module
 * @details It communicates clock frequencies in Hz
 */
struct pmc_data {
  uint32_t mck;
  uint32_t slck;
};

#endif /* _PMC_INTERFACE_H_ */