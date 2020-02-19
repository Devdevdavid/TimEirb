#ifndef PMC_REGISTERS__H
#define PMC_REGISTERS__H


#define PMC_BASE_ADDRESS (0x400E0600)
#define PMC_MAX_ADDRESS (PMC_BASE_ADDRESS+0x010C)
#define PMC_SCER_OFFSET (0)

#define CKGR_MOR_OFFSET (0x0020)
#define PMC_MCKR_OFFSET (0x0030)
#define PMC_PCER0_OFFSET (0x0010)
#define PMC_PCDR0_OFFSET (0x0014)





/*
	PMC_SCER offset 0x0000
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t r0: 5;
		uint32_t UOTGCLK :1;
		uint32_t r1: 2;
		uint32_t PCK0 :1;
		uint32_t PCK1 :1;
		uint32_t PCK2 :1;
		uint32_t r2: 21;
	}bits;
} PMC_SCER_reg;

/*
	PMC_SCDR offset 0x0004
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t r0: 5;
		uint32_t UOTGCLK :1;
		uint32_t r1: 2;
		uint32_t PCK0 :1;
		uint32_t PCK1 :1;
		uint32_t PCK2 :1;
		uint32_t r2: 21;
	}bits;
} PMC_SCDR_reg;

/*
	PMC_SCSR offset 0x0008
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t r0: 5;
		uint32_t UOTGCLK :1;
		uint32_t r1: 2;
		uint32_t PCK0 :1;
		uint32_t PCK1 :1;
		uint32_t PCK2 :1;
		uint32_t r2: 21;
	}bits;
} PMC_SCSR_reg;
#define PMC_SCSR_RESET_VALUE (0x0000_0001)

/*
	PMC_PCER0 offset 0x0010
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t r0: 2;
		uint32_t PID2: 1;
		uint32_t PID3: 1;
		uint32_t PID4: 1;
		uint32_t PID5: 1;
		uint32_t PID6: 1;
		uint32_t PID7: 1;
		uint32_t PID8: 1;
		uint32_t PID9: 1;
		uint32_t PID10: 1;
		uint32_t PID11: 1;
		uint32_t PID12: 1;
		uint32_t PID13: 1;
		uint32_t PID14: 1;
		uint32_t PID15: 1;
		uint32_t PID16: 1;
		uint32_t PID17: 1;
		uint32_t PID18: 1;
		uint32_t PID19: 1;
		uint32_t PID20: 1;
		uint32_t PID21: 1;
		uint32_t PID22: 1;
		uint32_t PID23: 1;
		uint32_t PID24: 1;
		uint32_t PID25: 1;
		uint32_t PID26: 1;
		uint32_t PID27: 1;
		uint32_t PID28: 1;
		uint32_t PID29: 1;
		uint32_t PID30: 1;
		uint32_t PID31: 1;
	}bits;
} PMC_PCER0_reg;

/*
	PMC_PCDR0 offset 0x0014
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t r0: 2;
		uint32_t PID2: 1;
		uint32_t PID3: 1;
		uint32_t PID4: 1;
		uint32_t PID5: 1;
		uint32_t PID6: 1;
		uint32_t PID7: 1;
		uint32_t PID8: 1;
		uint32_t PID9: 1;
		uint32_t PID10: 1;
		uint32_t PID11: 1;
		uint32_t PID12: 1;
		uint32_t PID13: 1;
		uint32_t PID14: 1;
		uint32_t PID15: 1;
		uint32_t PID16: 1;
		uint32_t PID17: 1;
		uint32_t PID18: 1;
		uint32_t PID19: 1;
		uint32_t PID20: 1;
		uint32_t PID21: 1;
		uint32_t PID22: 1;
		uint32_t PID23: 1;
		uint32_t PID24: 1;
		uint32_t PID25: 1;
		uint32_t PID26: 1;
		uint32_t PID27: 1;
		uint32_t PID28: 1;
		uint32_t PID29: 1;
		uint32_t PID30: 1;
		uint32_t PID31: 1;
	}bits;
} PMC_PCDR0_reg;

/*
	PMC_PCSR0 offset 0x0018
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t r0: 2;
		uint32_t PID2: 1;
		uint32_t PID3: 1;
		uint32_t PID4: 1;
		uint32_t PID5: 1;
		uint32_t PID6: 1;
		uint32_t PID7: 1;
		uint32_t PID8: 1;
		uint32_t PID9: 1;
		uint32_t PID10: 1;
		uint32_t PID11: 1;
		uint32_t PID12: 1;
		uint32_t PID13: 1;
		uint32_t PID14: 1;
		uint32_t PID15: 1;
		uint32_t PID16: 1;
		uint32_t PID17: 1;
		uint32_t PID18: 1;
		uint32_t PID19: 1;
		uint32_t PID20: 1;
		uint32_t PID21: 1;
		uint32_t PID22: 1;
		uint32_t PID23: 1;
		uint32_t PID24: 1;
		uint32_t PID25: 1;
		uint32_t PID26: 1;
		uint32_t PID27: 1;
		uint32_t PID28: 1;
		uint32_t PID29: 1;
		uint32_t PID30: 1;
		uint32_t PID31: 1;
	}bits;
} PMC_PCSR0_reg;
#define PMC_PCSR0_RESET_VALUE (0)

/*
	CKGR_UCKR offset 0x001C
*/


/*
	CKGR_MOR offset 0x0020
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t MOSCXTEN: 1;
		uint32_t PMOSCXTBY: 1;
		uint32_t r0: 1;
		uint32_t MOSCRCEN: 1;
		uint32_t MOSCRCF: 3;
		uint32_t r1: 1;
		uint32_t MOSCXTST: 8;
		uint32_t KEY: 8;
		uint32_t MOSCSEL: 1;
		uint32_t CFDEN: 1;
		uint32_t r2: 6;		
	}bits;
} CKGR_MOR_reg;

/*
	CKGR_MCFR offset 0x0024
*/


/*
	CKGR_PLLAR offset 0x0028
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t DIVA: 8;
		uint32_t PLLACOUNT: 6;
		uint32_t r0: 2;
		uint32_t MULA: 11;
		uint32_t r1: 2;
		uint32_t ONE: 1;
		uint32_t r2: 2;	
	}bits;
} CKGR_PLLAR_reg;

/*
	PMC_MCKR offset 0x0030
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t CSS: 2;
		uint32_t r0: 2;
		uint32_t PRES: 3;
		uint32_t r1: 5;
		uint32_t PLLADIV2: 1;
		uint32_t UPLLDIV2: 1;
		uint32_t r2: 18;	
	}bits;
} PMC_MCKR_reg;

/*
	PMC_USB offset 0x0038
*/

/*
	PMC_PCKx offset 0x0040
*/

/*
	PMC_IDR offset 0x0030
*/

/*
	PMC_IER offset 0x0030
*/

/*
	PMC_PCER1 offset 0x0100
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t PID31: 1;
		uint32_t PID32: 1;
		uint32_t PID33: 1;
		uint32_t PID34: 1;
		uint32_t PID35: 1;
		uint32_t PID36: 1;
		uint32_t PID37: 1;
		uint32_t PID38: 1;
		uint32_t PID39: 1;
		uint32_t PID40: 1;
		uint32_t PID41: 1;
		uint32_t PID42: 1;
		uint32_t PID43: 1;
		uint32_t PID44: 1;
		uint32_t r0: 18;
	}bits;
} PMC_PCER1_reg;

/*
	PMC_PCDR1 offset 0x0104
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t PID31: 1;
		uint32_t PID32: 1;
		uint32_t PID33: 1;
		uint32_t PID34: 1;
		uint32_t PID35: 1;
		uint32_t PID36: 1;
		uint32_t PID37: 1;
		uint32_t PID38: 1;
		uint32_t PID39: 1;
		uint32_t PID40: 1;
		uint32_t PID41: 1;
		uint32_t PID42: 1;
		uint32_t PID43: 1;
		uint32_t PID44: 1;
		uint32_t r0: 18;
	}bits;
} PMC_PCDR1_reg;

/*
	PMC_PCSR1 offset 0x0108
*/
typedef union {
	uint32_t integer_value;
	
	struct {
		uint32_t PID31: 1;
		uint32_t PID32: 1;
		uint32_t PID33: 1;
		uint32_t PID34: 1;
		uint32_t PID35: 1;
		uint32_t PID36: 1;
		uint32_t PID37: 1;
		uint32_t PID38: 1;
		uint32_t PID39: 1;
		uint32_t PID40: 1;
		uint32_t PID41: 1;
		uint32_t PID42: 1;
		uint32_t PID43: 1;
		uint32_t PID44: 1;
		uint32_t r0: 18;
	}bits;
} PMC_PCSR1_reg;






#endif // PMC_REGISTERS__H