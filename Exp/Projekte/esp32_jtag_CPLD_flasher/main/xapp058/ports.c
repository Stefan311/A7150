/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/* Revisions:                                          */
/* 12/01/2008:  Same code as before (original v5.01).  */
/*              Updated comments to clarify instructions.*/
/*              Add print in setPort for xapp058_example.exe.*/
/*******************************************************/
#include "ports.h"

#include <unistd.h>
#include <driver/gpio.h>

uint8_t *xsvf_data;
uint32_t xsvf_size;
uint32_t xsvf_offset;

#define PIN_TDI    41
#define PIN_TDO    13
#define PIN_TCK    45
#define PIN_TMS    39

/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void setPort(short p,short val)
{
    if (p==TCK) gpio_set_level(PIN_TCK,val);
    if (p==TMS) gpio_set_level(PIN_TMS,val);
    if (p==TDI) gpio_set_level(PIN_TDI,val);
}

/* toggle tck LH.  No need to modify this code.  It is output via setPort. */
/* toggle tck LHL */
void pulseClock()
{
    gpio_set_level(PIN_TCK,0);
    gpio_set_level(PIN_TCK,1);
}

/* readByte:  Implement to source the next byte from your XSVF file location */
/* read in a byte of data from the prom */
void readByte(unsigned char *data)
{
    if (xsvf_offset>=xsvf_size)
    {
        *data = 0;
    }
    else
    {
        *data = xsvf_data[xsvf_offset];
        xsvf_offset++;
    }
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
unsigned char readTDOBit()
{
    return gpio_get_level(PIN_TDO)==0 ? 0 : 1;
}

void waitTime(long microsec)
{
    usleep(microsec);
}
