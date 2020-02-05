#include "Testbench.h"
#include "tlm_head.h"

int sc_main(int argc, char *argv[]) {
  Testbench Testbench("top_testbench");

  sc_start();
  return 0;
}
