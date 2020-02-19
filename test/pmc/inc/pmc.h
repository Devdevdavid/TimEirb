#ifndef PMC__H
#define PMC__H

#include "pmc_registers.h"

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "data_clock.h"

enum MAINCK_SRC {
	internal_RC =0,
	external_xtal
}; 

enum INTERNAL_RC_FREQ {
	_4M =0,
	_8M,
	_12M
};

enum MCK_SRC {
	SLCK =0,
	MAINCK,
	PLLACK,
	UPLLCK
}; 

typedef enum {
	DIV_1 =0,
	DIV_2,
	DIV_4,
	DIV_8,
	DIV_16,
	DIV_32,
	DIV_64,
	DIV_3
} PRESCALER_VALUE;
	

SC_MODULE(PMC)
{
	
	tlm_utils::simple_initiator_socket<PMC> TimerSocket;
	tlm_utils::simple_target_socket<PMC> memory_socket;
	
	SC_CTOR(PMC);
	virtual void b_transport_memory( tlm::tlm_generic_payload& trans, sc_time& delay );
	
	void sendSocket();
	
	void set_external_xtal_freq(uint32_t freq);
	
	private :
		DataClock uartData, timerData, gpioData;
		uint32_t externalXtalFreq;
		uint32_t  freqMainck; 
		MAINCK_SRC SrcMainck; 
		INTERNAL_RC_FREQ freqRc;
	
		PRESCALER_VALUE PMC_MCKR_prescaler;
		PRESCALER_VALUE PMC_PCK_prescaler;
	
		MCK_SRC MCK_clk_src;
		PRESCALER_VALUE MCK_clk_prescaler;
	
		union {
			uint32_t reg_array[77];
			
			struct {
				PMC_SCER_reg PMC_SCER;
				PMC_SCDR_reg PMC_SCDR;
				PMC_SCSR_reg PMC_SCSR; 
				uint32_t r0;
				PMC_PCER0_reg PMC_PCER0;
				PMC_PCDR0_reg PMC_PCDR0;
				PMC_PCSR0_reg PMC_PCSR0;
				uint32_t CKGR_UCKR_unsupported;
				CKGR_MOR_reg  CKGR_MOR; 
				uint32_t  CKGR_MCFR_unsupported;
				CKGR_PLLAR_reg CKGR_PLLAR;
				uint32_t r1;
				PMC_MCKR_reg PMC_MCKR; 
				uint32_t r2[51];
				PMC_PCER1_reg PMC_PCER1;
				PMC_PCDR1_reg PMC_PCDR1;
				PMC_PCSR1_reg PMC_PCSR1;
			} reg_struct;
		}PMC_REGS; 
		
		// Array of N peripherals : 0, the periph is desactivated, 1 it is activated
		uint8_t uart_activated[4];
		uint8_t pio_activated[6];
		uint8_t timer_activated[9]; 
		
		void update(void);
	

};
#endif // PMC__H
