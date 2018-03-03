#ifndef IO
#define IO

#include <stdint.h>
#include "/opt/mcb32tools/include/pic32mx.h"

 volatile int* trisd = ((int*) 0xbf8860c0);

int get_sw( void ) {
	int retVal = ((int*) 0xbf8860c0)[ 4 ] >> 8;
    return( retVal & 15 );
}
int get_btns(void) {
	int retVal = ((int*) 0xbf8860c0)[ 4 ] >> 4;
	retVal &= ~1;
	retVal |= PORTF >> 1 & 1;
	return( retVal & 0xf );
}
#endif