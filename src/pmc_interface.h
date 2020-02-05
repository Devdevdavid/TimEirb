#ifndef _PMC_INTERFACE_H_
#define _PMC_INTERFACE_H_

#include <cstdint>

struct pmc_data {
  uint32_t mck;
  uint32_t slck;
};

#endif /* _PMC_INTERFACE_H_ */