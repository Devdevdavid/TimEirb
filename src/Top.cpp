#include "Top.h"

Top::Top(sc_module_name name) : sc_module(name)
{
	initiator = new Initiator("initiator");
	memory    = new Memory   ("memory");

	initiator->socket.bind(memory->socket);
}
