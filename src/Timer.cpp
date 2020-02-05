#include "Timer.h"
#include "channel.h"

Timer::Timer(sc_module_name name) :  sc_module(name), socket_PMC("socket_PMC"), socket_Bus("socket_Bus") {
  
channel_1 = new Channel("channel");
//  channel_2 = new channel("channel");
//  channel_3 = new channel("channel");

  socket_PMC.register_b_transport(this, &Timer::b_transport_pcm);
  socket_Bus.register_b_transport(this, &Timer::b_transport_bus);

}



void Timer::b_transport_pcm(tlm_generic_payload& trans, sc_time& delay)
  {
	
}

void Timer::b_transport_bus(tlm_generic_payload& trans, sc_time& delay)
  {
	
}
