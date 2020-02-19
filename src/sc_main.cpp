#include "Testbench.h"
#include "pmc.h"
#include "tlm_head.h"

int sc_main(int argc, char *argv[]) {
  Testbench Testbench("top_testbench");
  PMC pmc("pmc");
  pmc.TimerSocket.bind(Testbench.timer1->socketPMC);
  Testbench.configSocket.bind(pmc.memory_socket);
  sc_start(10, SC_SEC);
  return 0;
}
