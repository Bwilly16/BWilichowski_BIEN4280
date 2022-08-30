#include "BWilichowski_binaryutils.h";

void setbit(uint32_t* addr, uint8_t whichbit)
{
	uint32_t* addr;
	uint8_t whichbit;
	uint32_t ans = 0;
	
	uint32_t temp = 1 << whichbit;
	ans = *addr | temp;
	*addr = ans;		
}

void clearbit(uint32_t* addr, uint8_t whichbit)
{
	uint32_t* addr;
	uint8_t whichbit;
	uint32_t ans = 0;

	uint32_t temp = 1 << whichbit;
	ans = *addr & ~temp;
	*addr = ans;
}

void setbits(uint32_t* addr, uint8_t bitmask)
{

}

void clearbits(uint32_t* addr, uint8_t bitmask)
{
	
}

void display_binary(uint32_t num)
{

}