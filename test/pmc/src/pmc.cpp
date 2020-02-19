#include <iostream>
#include <cassert>
#include "pmc.h"

#include "pmc_registers.h"

/*Socket memory*/

PMC::PMC(sc_module_name name) : TimerSocket("TIMERSocket")
{
	memory_socket.register_b_transport(this, &PMC::b_transport_memory); // pour enregistrer une cb
	
	this->externalXtalFreq = 10;

	this->freqRc = _4M;
	this->SrcMainck = internal_RC;

	this->MCK_clk_src = SLCK;
	this->MCK_clk_prescaler = DIV_1;

	for (uint32_t i = 0; i < 4; i++)
		this->uart_activated[i] = 0;

	for (uint32_t i = 0; i < 6; i++)
		this->pio_activated[i] = 0;

	for (uint32_t i = 0; i < 9; i++)
		this->timer_activated[i] = 0;

	SC_THREAD(update);
}

void PMC::set_external_xtal_freq(uint32_t freq)
{
	this->externalXtalFreq = freq;
}

int is_reg_readable(uint32_t offset)
{
	uint32_t readable[] = {0x0008, 0x0018, 0x001C, 0x0020, 0x0024, 0x0028, 0x0030, 0x0038, 0x0040, 0x0044, 0x0048, 0x0068, 0x006C, 0x0070, 0x0074, 0x00E4, 0x00E8, 0x0108, 0x010C};
	// 19 registres accessibles en lecture
	for (uint8_t idx = 0; idx < 19; idx++)
	{
		if (readable[idx] == offset)
			return 1;
	}

	return 0;
}

void PMC::b_transport_memory(tlm::tlm_generic_payload &trans, sc_time &delay)
{
	tlm::tlm_command cmd = trans.get_command();
	sc_dt::uint64 adr = trans.get_address();
	unsigned char *ptr = trans.get_data_ptr();
	unsigned int len = trans.get_data_length();
	unsigned char *byt = trans.get_byte_enable_ptr();
	unsigned int wid = trans.get_streaming_width();

	// Check if the PMC peripherial is not addressed
	if (adr > PMC_MAX_ADDRESS || adr < PMC_BASE_ADDRESS)
	{
		printf("[PMC] Ignored packet at address %llX.\n", adr);
		return;
	}

	// Compute register offset from address
	uint64_t offset = adr - PMC_BASE_ADDRESS;

	// Convert register offset to index in our register array
	uint32_t register_index = offset / 4;

	// Pointer to the register targeted by address
	uint8_t *preg = (uint8_t *)(&(this->PMC_REGS.reg_array[register_index]));

	if (cmd == tlm::TLM_READ_COMMAND)
	{
		// Check if register can be read
		if (!is_reg_readable(offset))
		{
			printf("[PMC] Register at offset %lX is not readable.\n", offset);
			trans.set_response_status( tlm::TLM_GENERIC_ERROR_RESPONSE );
			return;
		}
		printf("[PMC] Reading %d bytes at address %llX.\n", len, adr);
		for (uint32_t i = 0; i < len; i++)
			printf("%X ", preg[i]);

		printf("\n");
		memcpy(ptr, preg, len);
		trans.set_response_status( tlm::TLM_OK_RESPONSE );
	}

	if (cmd == tlm::TLM_WRITE_COMMAND)
	{
		// TODO : check if register can be written

		assert(len <= 4);

		memcpy(preg, ptr, len);

		if (offset == CKGR_MOR_OFFSET)
		{
			CKGR_MOR_reg *reg = (CKGR_MOR_reg *)ptr;

			this->SrcMainck = static_cast<MAINCK_SRC>(reg->bits.MOSCSEL);
			this->freqRc = static_cast<INTERNAL_RC_FREQ>(reg->bits.MOSCRCF);
			printf("[PMC] Updating CKGR_MOR: new value %X.\n", this->PMC_REGS.reg_struct.CKGR_MOR.integer_value);
		}
		else if (offset == PMC_MCKR_OFFSET)
		{
			PMC_MCKR_reg *reg = (PMC_MCKR_reg *)ptr;

			this->MCK_clk_src = static_cast<MCK_SRC>(reg->bits.CSS);
			this->MCK_clk_prescaler = static_cast<PRESCALER_VALUE>(reg->bits.PRES);
			printf("[PMC] Updating PMC_MCKR: new value %X.\n", this->PMC_REGS.reg_struct.PMC_MCKR.integer_value);
		}
		else if (offset == PMC_PCER0_OFFSET)
		{
			PMC_PCER0_reg *reg = (PMC_PCER0_reg *)ptr;

			uint32_t val = reg->integer_value;

			// First, PIO (PID 11 to 16)
			val = val >> 11;
			for (uint32_t i = 0; i < 6; i++)
			{
				if (val & 1)
				{
					this->pio_activated[i] = 1;
					printf("[PMC] Activating PIO %d.\n", i);
				}

				val = val >> 1;
			}
			// Next, UART (PID 17 to 20)
			for (uint32_t i = 0; i < 4; i++)
			{
				if (val & 1)
				{
					this->uart_activated[i] = 1;
					printf("[PMC] Activating UART %d.\n", i);
				}
				val = val >> 1;
			}

			// Next, Timer (PID 27 to 35)
			val = val >> 7;
			for (uint32_t i = 0; i < 9; i++)
			{
				if (val & 1)
				{
					this->timer_activated[i] = 1;
					printf("[PMC] Activating Timer %d.\n", i);
				}
				val = val >> 1;
			}
		}
		else if (offset == PMC_PCDR0_OFFSET)
		{
			PMC_PCDR0_reg *reg = (PMC_PCDR0_reg *)ptr;

			uint32_t val = reg->integer_value;

			// First, PIO (PID 11 to 16)
			val = val >> 11;
			for (uint32_t i = 0; i < 6; i++)
			{
				if (val & 1)
				{
					this->pio_activated[i] = 0;
					printf("[PMC] Disabling PIO %d.\n", i);
				}

				val = val >> 1;
			}
			// Next, UART (PID 17 to 20)
			for (uint32_t i = 0; i < 4; i++)
			{
				if (val & 1)
				{
					this->uart_activated[i] = 0;
					printf("[PMC] Disabling UART %d.\n", i);
				}
				val = val >> 1;
			}

			// Next, Timer (PID 27 to 35)
			val = val >> 7;
			for (uint32_t i = 0; i < 9; i++)
			{
				if (val & 1)
				{
					this->timer_activated[i] = 0;
					printf("[PMC] Disabling Timer %d.\n", i);
				}
				val = val >> 1;
			}
		}

		this->update();
		trans.set_response_status( tlm::TLM_OK_RESPONSE );
	}
}

void PMC::update(void)
{
	uint32_t prescaler = 1;
	uint32_t clkOrigin = 1;

	switch (MCK_clk_prescaler)
	{
	case (DIV_1):
		prescaler = 1;
		break;
	case (DIV_2):
		prescaler = 2;
		break;
	case (DIV_4):
		prescaler = 4;
		break;
	case (DIV_8):
		prescaler = 8;
		break;
	case (DIV_16):
		prescaler = 16;
		break;
	case (DIV_32):
		prescaler = 32;
		break;
	case (DIV_64):
		prescaler = 64;
		break;
	case (DIV_3):
		prescaler = 3;
		break;
	}
	if (this->MCK_clk_src == SLCK)
		clkOrigin = 32000;
	else if (this->MCK_clk_src == MAINCK)
	{
		if (this->SrcMainck == internal_RC)
		{
			if (this->freqRc == _4M)
				clkOrigin = 4000000;
			else if (this->freqRc == _8M)
				clkOrigin = 8000000;
			else if (this->freqRc == _12M)
				clkOrigin = 12000000;
		}
		else
		{
			clkOrigin = this->externalXtalFreq;
		}
	}

	this->uartData.mck = clkOrigin / prescaler;
	this->uartData.slck = 32000;

	this->gpioData.mck = clkOrigin / prescaler;
	this->gpioData.slck = 32000;

	this->timerData.mck = clkOrigin / prescaler;
	this->timerData.slck = 32000;

	if (this->uart_activated[0] == 0)
	{
		this->uartData.mck = 0;
		this->uartData.slck = 0;
	}

	if (this->timer_activated[0] == 0)
	{
		this->timerData.mck = 0;
		this->timerData.slck = 0;
	}

	if (this->pio_activated[0] == 0)
	{
		this->gpioData.mck = 0;
		this->gpioData.slck = 0;
	}

	this->sendSocket();
}

void PMC::sendSocket()
{
	tlm::tlm_generic_payload *trans_UART = new tlm::tlm_generic_payload;
	tlm::tlm_generic_payload *trans_GPIO = new tlm::tlm_generic_payload;
	tlm::tlm_generic_payload *trans_TIMER = new tlm::tlm_generic_payload;
	sc_time delay = sc_time(10, SC_NS);

	trans_UART->set_data_ptr(reinterpret_cast<unsigned char *>(&uartData));
	trans_UART->set_data_length(sizeof(DataClock));
	trans_UART->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

	trans_GPIO->set_data_ptr(reinterpret_cast<unsigned char *>(&gpioData));
	trans_GPIO->set_data_length(sizeof(DataClock));
	trans_GPIO->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

	trans_TIMER->set_data_ptr(reinterpret_cast<unsigned char *>(&timerData));
	trans_TIMER->set_data_length(sizeof(DataClock));
	trans_TIMER->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

	//UartSocket->b_transport(*trans_UART, delay);
	//GpioSocket->b_transport(*trans_GPIO, delay);
	TimerSocket->b_transport(*trans_TIMER, delay);

	//if (trans_UART->is_response_error())
	//	SC_REPORT_ERROR("TLM-2", "Response error from trans_UART");

	//if (trans_GPIO->is_response_error())
	//	SC_REPORT_ERROR("TLM-2", "Response error from trans_GPIO");

	if (trans_TIMER->is_response_error())
		SC_REPORT_ERROR("TLM-2", "Response error from trans_TIMER");
}
