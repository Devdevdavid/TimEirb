#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <cstdint>

// MACROS
#define is_in_range(value, start, len)  ((value >= start) && (value < (start + len)))
#define _need_wpen_()    				if (this->isWriteProtected) { return -1; } // Return if write permission is off

// For cmd testing
#define _is_write()			(cmd == TLM_WRITE_COMMAND)
#define _is_read()			(cmd == TLM_READ_COMMAND)
#define _is_write_only_()	if (!_is_write()) { return -1; }
#define _is_read_only_()	if (!_is_read()) { return  -1; }
#define _is_read_write_()

// CONSTANTS
#define KILO			(1000)
#define MEGA			(KILO * KILO)
#define GIGA			(KILO * MEGA)
#define REGISTER_SIZE	sizeof(uint32_t)

int64_t get_min_pos_value(int64_t array[], int len);


#endif /* _TOOLS_H_ */