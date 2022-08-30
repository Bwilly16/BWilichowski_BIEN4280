// Author: Ben Wilichowski
// Date Created: 8/30/2022
// What it does: 
#ifndef VARNAME
#define VARNAME
#include <stdint.h>
#include <stdio.h> 

//Declare all required functions
void setbit(uint32_t* addr, uint8_t whichbit);
void clearbit(uint32_t* addr, uint8_t whichbit);
void setbits(uint32_t* addr, uint8_t bitmask);
void clearbits(uint32_t* addr, uint8_t bitmask);
void display_binary(uint32_t num);

#endif VARNAME

