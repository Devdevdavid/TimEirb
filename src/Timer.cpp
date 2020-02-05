#include "Timer.h"
#include "Channel.h"

Timer::Timer(sc_module_name name) : sc_module(name){
  channel_1 = new channel("channel");
//channel_2 = new channel("channel");
//  channel_3 = new channel("channel");

void Timer::b_transport(tlm_generic_payload& trans, sc_time& delay)
{
  tlm_command      cmd = trans.get_command();
  sc_dt::uint64    adr = trans.get_address() / 4;
  unsigned char*   ptr = trans.get_data_ptr();
  unsigned int     len = trans.get_data_length();
  unsigned char*   byt = trans.get_byte_enable_ptr();
  unsigned int     wid = trans.get_streaming_width();

  if (adr >= MEMORY_MAX_SIZE || byt != 0 || (adr + len) >= MEMORY_MAX_SIZE) {
    SC_REPORT_ERROR("TLM-2", "Timer does not support given generic payload transaction");
  }

  if (cmd == TLM_READ_COMMAND) {
    memcpy(ptr, &mem[adr], len);
  }
  else if (cmd == TLM_WRITE_COMMAND) {
    memcpy(&mem[adr], ptr, len);
  }

  trans.set_response_status(TLM_OK_RESPONSE)

  socket_PMC.bind(channel->socket);
  socket_Bus.bind(channel->socket);

}
