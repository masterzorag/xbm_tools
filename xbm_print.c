/*
	gcc -O2 -Wall -o xbm_print xbm_print.c

# 1. rebuild razors.ttf/xbm/razors.ttf.h
# ./genXBMfonts.sh

# 2. use generated XBM fonts: hardcode in xbm_print
# cp razors.ttf/xbm/razors.ttf.h xbm_font.h

# 3. rebuild xbm_tools
# make clean && make

# ./xbm_print mz
109 [m]
     ****     **
      **      **
              **
    *    *    **
    **  **    **
    ******    **
     *****    **
      ****    **
      ****    **
      ****    **
      ****    **
      ****    **
      ****    **
      ****    **
****************
****************
122 [z]
              **
              **
**********    **
**********    **
**********    **
**********    **
**           ***
*           ****
     ***********
     ***********
              **
              **
              **
              **
****************
****************

*/

#include <stdio.h>
#include "xbm_font.h" /* ! generate it with my genXBMfonts script ! */

void xbm_print(const int x, const int y, const char *text, unsigned int *buffer)
{
	int i, j;
//	int tempx = 0; // scans the font bitmap, j++ 0-7: +j
	int tempy = 0;
	char c;

	while(*text != '\0'){
		c = *text;

		if(c < LOWER_ASCII_CODE || c > UPPER_ASCII_CODE) c = 180;
		printf("%d [%c]\n", c, c);
		
		// font indexing by current char
		char *bit = xbmFont[c - LOWER_ASCII_CODE];
		
		// dump bits map: bytes_per_line 2, size 32 char of 8 bit
		for(i = 0; i < 32; i++) {
			// scans 8 bits per char
			for(j = 0; j < 8; j++){
			//	printf("%c", (data[i] & (1 << j)) ? ' ' : '0' );		// least significant bit first
			//	printf("%d", (data[i] & (0x80 >> j)) ? 1 : 0); 			// right shifting the value will print bits in reverse.				
				if(bit[i] & (1 << j)){		// least significant bit first
					printf(" ");
				} else { // paint pixel
					printf("*");
				}
			}
			
	  		if(i % (FONT_W /8) != 0) {
	  			puts("");	//decrease gradient
	  			tempy++;
			}
		}
		tempy = 0;	
		++text;
	}
}

int main(int argc, char **argv) {
	if(argv[1]){
		xbm_print(0, 0, argv[1], NULL);
		return 1;	
	}
	
	return 0;
}	
