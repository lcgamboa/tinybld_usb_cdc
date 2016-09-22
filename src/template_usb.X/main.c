//Sample source code for test USB-CDC tiny bootloader firmware

//for sdcc program memory range in lkr file must be 0x0000 at 0x7177

//for XC8 use range 0000-7177 in XC8-Linke ROM ranges option

#ifdef __XC8
#include <xc.h>
#else 
#include "pic18fregs.h"
#endif

#include "config.h"


void main(void) {
    
    unsigned long x;
    TRISB=0;
    
    while(1)
    {
    PORTB=0;
    for(x=0;x<650000;x++);
    PORTB=0xFF;
    for(x=0;x<650000;x++);

    }

}
