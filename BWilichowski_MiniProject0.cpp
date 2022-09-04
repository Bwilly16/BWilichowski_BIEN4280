#include "BWilichowski_binaryutils.h"


int main() 
{
	uint32_t solo = 0; //Define input binary variable set to all 0's
	
	setbit(&solo, 24); //Set the 24th bit
	printf("Set 24th bit: \n");
	display_binary(solo);
	

	setbit(&solo, 16); //Set the 16th bit
	printf("\nSet 16th bit: \n");
	display_binary(solo);
	

	setbit(&solo, 17); //Set the 17th bit
	printf("\nSet 17th bit: \n");
	display_binary(solo);
	

	setbits(&solo, 4095); //Set bits 0-11
	printf("\nSet 0-11th bits: \n");
	display_binary(solo);
	

	clearbit(&solo, 11); //Clear bit 11
	printf("\nClear 11th bit: \n");
	display_binary(solo);
	

	clearbits(&solo, 240);//240 = 11110000
	printf("\nBinary Solo:\n\r");
	display_binary(solo);

	return(0);
}