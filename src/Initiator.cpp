#include "Initiator.h"

Initiator::Initiator(sc_module_name name) : socket("socket")
{
	SC_THREAD(thread_process);
}

void Initiator::thread_process(void)
{
	uint8_t dataW[10];
	uint8_t dataR[10];
	uint8_t adr = 0;

	tlm_generic_payload* trans = new tlm_generic_payload;
	sc_time delay = sc_time(10, SC_NS);

	// Prepare data for a write command
	for (int i = 0; i < sizeof(dataW); ++i) {
		dataW[i] = i;
	}

	trans->set_command(TLM_WRITE_COMMAND);
	trans->set_address(adr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&dataW));
	trans->set_data_length(sizeof(dataW));
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

	// Send it !
	socket->b_transport(*trans, delay);

	if (trans->is_response_error()) {
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
	}

	// Read
	trans->set_command(TLM_READ_COMMAND);
	trans->set_address(adr);
	trans->set_data_ptr(reinterpret_cast<unsigned char*>(&dataR));
	trans->set_data_length(sizeof(dataR));
	trans->set_byte_enable_ptr(0);
	trans->set_dmi_allowed(false);
	trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

	// Send it !
	socket->b_transport(*trans, delay);

	if (trans->is_response_error()) {
		SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
	}

	// Compare
	for (int i = 0; i < sizeof(dataR); ++i) {
		if (dataR[i] != dataW[i]) {
			cerr << "Read/Write are different !" << endl;
			cerr << "Write: ";
			print_buffer(dataW, sizeof(dataW));
			cerr << "Read: ";
			print_buffer(dataR, sizeof(dataR));
		}
	}
}

void Initiator::print_buffer(uint8_t data[], uint16_t len)
{
	for (int i = 0; i < len; ++i) {
		printf(" 0x%02X", data[i]);
	}
	printf("\n");
}