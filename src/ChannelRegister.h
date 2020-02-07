#ifndef _CHANNEL_REGISTER_H_
#define _CHANNEL_REGISTER_H_

#define TC_CCR        		0
#define TC_CMR        		1
#define TC_SMMR       		2
#define TC_RESERVED   		3
#define TC_CV         		4
#define TC_RA         		5
#define TC_RB         		6
#define TC_RC         		7
#define TC_SR         		8
#define TC_IER        		9
#define TC_IDR        		10
#define TC_IMR        		11
#define TC_RESERVED_2 		12
#define TC_RESERVED_3 		13
#define TC_RESERVED_4 		14
#define TC_RESERVED_5 		15

// TC_CMR
#define TCCLKS 	0x00000007
#define CPCSTOP 0x00000040
#define CPCDIS	0x00000080

// TC_CCR
#define CLKDIS	0x00000002
#define CLKEN	0x00000001

#endif /* _CHANNEL_REGISTER_H_ */
