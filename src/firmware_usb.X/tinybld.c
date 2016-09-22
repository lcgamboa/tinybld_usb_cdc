/*
 * USB-CDC bootloader compatible with Tiny Bootloader
 *
Copyright (c) 2016 Luis Claudio Gambôa Lopes
 
based on Tiny Bootloader firmware
 of claudiu.chiculita@ugal.ro
 http://www.etc.ugal.ro/cchiculita/software/picbootloader.htm
  
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include "pic18f4550.h"
#include"config.h"
#include "usbcdc.h"
#include "usb_defs.h"
                        //must be aligned with 64 boundary 
//The same of lkr file    bootloader size = 3720 = 0x0E88         
#define first_address  0x7178  //0x8000-0x0E80
#pragma code space 0x7178 //#pragma code space 0x7180 //



#define IdTypePIC 0x55
//#define max_flash 0x8000
#define xtal 48000000           
#define spbrg_value 25 //115200

unsigned char c;//return of Receive
unsigned char crc=0;
unsigned char flag;
unsigned char cnt1,cnt2,cnt3;
unsigned char i;
unsigned char *ptr;
unsigned char counter_hi;
unsigned char counter_lo;

unsigned char buffer[65];

unsigned long ct;

//space to alloc inital goto
void space()__naked
{
    __asm
    GOTO 0x0100
    NOP
    NOP
    __endasm;
}

//sample program
#pragma code user 0x0100
void user()
{
    unsigned int x;
    TRISB=0;
    
    while(1)
    {
    PORTB=0;
    for(x=0;x<65000;x++);
    PORTB=0xFF;
    for(x=0;x<65000;x++);
    }
}


//;******** procedures ******************

void Write(unsigned char cfg)
{
	EECON1=cfg;
	EECON2=0x55;
	EECON2=0xAA;
	EECON1bits.WR=1;	//;WRITE
 }


void Receive(void)
{
// for 20MHz => 11 => 1second delay
for(cnt1=0;cnt1<xtal/8000000+1;cnt1++)    
  for(cnt2=0;cnt2 < 255; cnt2++)    
    for(cnt3=0;cnt3 < 255; cnt3++ )
    {
        usbcdc_handler();
		if(usbcdc_rd_ready())//			;test RX
        {
          usbcdc_flush();
	      c=usbcdc_getchar();  
	      crc+=c;		//	;compute crc
		  return;
        }
    }
	//;timeout:
//way_to_exit

//	   bra first_address 
    UCONbits.USBEN=0;
    __asm
      GOTO first_address      
    __endasm;
}    
    
//position in lkr file
void main()
{ 
    usbcdc_init();
    
    usbcdc_handler();
  
    ct=0;        
    while (usbcdc_device_state != CONFIGURED)
    {
        usbcdc_handler();
        
        ct++;
        if (ct > 200000l) //delay to exit if no USB   
        {
	      //bra first_address 
          UCONbits.USBEN=0;
          __asm
            GOTO first_address      
          __endasm;
        }

    }
    
    
    //wait for computer
	Receive();//result in c var			
	if(c != 0xC1)  //				;Expect C1h
    {
//	   RCSTAbits.SPEN=0; //deactivate UART
//	   bra first_address
    UCONbits.USBEN=0;    
    __asm
       GOTO first_address      
    __endasm;
    }
    
	usbcdc_putchar(IdTypePIC);//				;send PIC type
    usbcdc_flush();
//MainLoop
while(1)
{    
	usbcdc_putchar('K');	//			; "-Everything OK, ready and waiting."
    usbcdc_flush();
//mainl
  while(1)
  {
	crc=0;
	Receive();  //Upper
	TBLPTRU=c;
    flag=c;		//(for EEPROM and CFG cases)
	Receive();  //Hi
	TBLPTRH=c;
    EEADR=c;    //(for EEPROM case)
	Receive();	//Lo
	TBLPTRL=c;
    EEDATA=c;   //(for EEPROM case)

	Receive();  //;count
	i=c;
	i++;
	//lfsr FSR0, (buffer-1)
    ptr=buffer;
//rcvoct						;read 64+1 bytes
    do{
      TABLAT=c;		//;prepare for cfg; => store byte before crc
	  Receive();
	  *ptr=c;
      ptr++;
	  i--;
    }
    while(i);        

	
	if(!crc)	//;check crc
    {
        INTCONbits.GIE = 0; 
		if(flag &0x40) //is EEPROM data?
		{
		  Write(0b00000100); //Setup eeprom
		  //bra waitwre
        }
        else if(flag & 0x80) //is CFG data
        {
          __asm  
		  tblwt*  
          __endasm; //;write TABLAT(byte before crc) to TBLPTR***
		  Write(0b11000100); //Setup cfg
		  //bra waitwre
        }
        else
        {

//noconfig and no eeprom
							//;write
//eraseloop
	Write(0b10010100);//Setup erase
	__asm
            TBLRD*-
    __endasm;        //; point to adr-1
	
//writebigloop			
	counter_hi=2; //2groups 
	//lfsr FSR0,buffer
    ptr=buffer;        
//writesloop
    do{
	  counter_lo=32;// 32bytes = 4instr
//writebyte
      do{
	    TABLAT=*ptr;// put 1 byte
        ptr++;
        __asm
	    tblwt+*
        __endasm;
	    counter_lo--;
        }
	  while(counter_lo);
	
	  Write(0b10000100); //Setup writes
	  counter_hi--;
	}
    while(counter_hi);
    }   
//waitwre	
	//btfsc EECON1,WR		;for eeprom writes (wait to finish write)
	//bra waitwre			;no need: round trip time with PC bigger than 4ms
	
	EECON1bits.WREN=0;			//;disable writes
	//bra MainLoop
	INTCONbits.GIE = 1; 
    break;
    }
    else
    {
//ziieroare					;CRC failed
	usbcdc_putchar('N');
    usbcdc_flush();
	//bra mainl
    }
  }  
}
    
    
/*    
;*************************************************************
; After reset
; Do not expect the memory to be zero,
; Do not expect registers to be initialised like in catalog.
*/


}