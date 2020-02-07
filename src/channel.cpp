#include "Channel.h"

Channel::Channel(sc_module_name name) : sc_module(name)
{

}
void Channel::generatedClockUpdate(void)
{
    switch (register_data[TC_CMR] & TCCLKS){
        case 0 :
            clk.generated_clock = clk.mclk/2;
        break;

        case 1 :
            clk.generated_clock = clk.mclk/8;
        break;

        case 2 :
            clk.generated_clock = clk.mclk/32;
        break;

        case 3 :
            clk.generated_clock = clk.mclk/128;
        break;

        case 4 :
            clk.generated_clock = clk.sclk;
        break;
    }
}

void Channel::UpdateClockCounter(void)
{
    if (clockEnable())
        clk.clock_counter = clk.generated_clock;
}

bool Channel::clockEnable(void)
{
    if (((register_data[TC_CCR] & CLKDIS) == (1 << 1)) 
     || ((register_data[TC_CM] & CPCSTOP) == (1 << 6)) 
     || ((register_data[TC_CCR] & CPCDIS) == (1 << 7)))
    {
        return false;
    }
    else if ((register_data[TC_CCR] & CLKEN) == 1)
    {
        return true;
    }
}

void Channel::tioUpdate(void)
{
}

void Channel::getTioa(struct tio tioa)
{
}