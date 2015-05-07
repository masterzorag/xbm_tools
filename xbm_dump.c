/*
	gcc -O2 -Wall -o xbm_dump xbm_dump.c

xbm_tools # ./xbm_dump ./mz.xbm 1
24x24   (-1 -1), data @0xbfecd298
size 72
bytes_per_line 3
      
                        
          ****          
          ****          
          ****          
          ****          
  ****    ****    ****  
  ****    ****    ****  
  ****    ****    ****  
  ****    ****    ****  
  ********************  
  ********************  
  ********************  
  ********************  
  ****            ****  
  ****            ****  
  ****            ****  
  ****            ****  
  ********    ********  
  ********    ********  
  ********    ********  
  ********    ********  
                        
                        
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* xbm parser borrowed from xc/lib/X11/RdBitF.c */
#define MAX_SIZE 255
#define TRUE 1
#define FALSE 0

/* shared data for the image read/parse logic */
static short hex_table[256];		/* conversion value */
static short initialized = FALSE;	/* easier to fill in at run time */

typedef unsigned char uchar;
typedef unsigned int uint;
typedef short bool;

/* Table index for the hex values. Initialized once, first time.
 * Used for translation value or delimiter significance lookup.
 */
static void
init_hex_table (void)
{
	/*
	 * We build the table at run time for several reasons:
	 *
	 * 1. portable to non-ASCII machines.
	 * 2. still reentrant since we set the init flag after setting table.
	 * 3. easier to extend.
	 * 4. less prone to bugs.
	 */
	hex_table['0'] = 0;
	hex_table['1'] = 1;
	hex_table['2'] = 2;
	hex_table['3'] = 3;
	hex_table['4'] = 4;
	hex_table['5'] = 5;
	hex_table['6'] = 6;
	hex_table['7'] = 7;
	hex_table['8'] = 8;
	hex_table['9'] = 9;
	hex_table['A'] = 10;
	hex_table['B'] = 11;
	hex_table['C'] = 12;
	hex_table['D'] = 13;
	hex_table['E'] = 14;
	hex_table['F'] = 15;
	hex_table['a'] = 10;
	hex_table['b'] = 11;
	hex_table['c'] = 12;
	hex_table['d'] = 13;
	hex_table['e'] = 14;
	hex_table['f'] = 15;

	/* delimiters of significance are flagged w/ negative value */
	hex_table[' '] = -1;
	hex_table[','] = -1;
	hex_table['}'] = -1;
	hex_table['\n'] = -1;
	hex_table['\t'] = -1;

	initialized = TRUE;
}

/* Read next hex value in the input stream, return -1 if EOF */
static int
next_int (FILE *fstream)
{
	int ch;
	int value = 0;
	int gotone = 0;
	int done = 0;
    
	/* loop, accumulate hex value until find delimiter 
	   skip any initial delimiters found in read stream */

	while (!done) {
		ch = getc (fstream);
		if (ch == EOF) {
			value = -1;
			done++;
		} else {
			/* trim high bits, check type and accumulate */
			ch &= 0xff;
			if (isascii (ch) && isxdigit (ch)) {
				value = (value << 4) + hex_table[ch];		
				gotone++;
			} else if ((hex_table[ch]) < 0 && gotone) {
				done++;
			}
		}
	}
	return value;
}


static bool
read_bitmap_file_data (FILE *fstream,
		       uint *width, uint *height,
		       uchar **data,
		       int *x_hot, int *y_hot)
{
	uchar *bits = NULL;		/* working variable */
	char line[MAX_SIZE];		/* input line from file */
	int size;			/* number of bytes of data */
	char name_and_type[MAX_SIZE];	/* an input line */
	char *type;			/* for parsing */
//	char *name;
	int value;			/* from an input line */
	int version10p;			/* boolean, old format */
	int padding;			/* to handle alignment */
	int bytes_per_line;		/* per scanline of data */
	uint ww = 0;			/* width */
	uint hh = 0;			/* height */
	int hx = -1;			/* x hotspot */
	int hy = -1;			/* y hotspot */

	/* first time initialization */
	if (!initialized) {
		init_hex_table ();
	}

	/* error cleanup and return macro */
#define	RETURN(code) { free (bits); return code; }

	while (fgets (line, MAX_SIZE, fstream)) {
		if (strlen (line) == MAX_SIZE-1)
			RETURN (FALSE);
		if (sscanf (line,"#define %s %d",name_and_type,&value) == 2) {
			if (!(type = strrchr (name_and_type, '_')))
				type = name_and_type;
			else {
				type++;
			}

			if (!strcmp ("width", type))
				ww = (unsigned int) value;
			if (!strcmp ("height", type))
				hh = (unsigned int) value;
			if (!strcmp ("hot", type)) {
				if (type-- == name_and_type
				    || type-- == name_and_type)
					continue;
				if (!strcmp ("x_hot", type))
					hx = value;
				if (!strcmp ("y_hot", type))
					hy = value;
			}
			continue;
		}
    
    
		if (sscanf (line, "static short %s = {", name_and_type) == 1)
			version10p = 1;
		else if (sscanf (line,"static unsigned char %s = {",name_and_type) == 1)
			version10p = 0;
		else if (sscanf (line, "static char %s = {", name_and_type) == 1)
			version10p = 0;
		else
			continue;

		
		if (!(type = strrchr (name_and_type, '_')))
			type = name_and_type;
		else
			type++;

		if (strcmp ("bits[]", type))
			continue;
					
		if (!ww || !hh)
			RETURN (FALSE);

		if ((ww % 16) && ((ww % 16) < 9) && version10p)
			padding = 1;
		else
			padding = 0;

		bytes_per_line = (ww+7)/8 + padding;

		size = bytes_per_line * hh;
		bits = malloc (size);

//D		printf("bytes_per_line %d, size %d, version10p %d\n", bytes_per_line, size, version10p);

		//		static char 037_bits[] = {
//D		name = strndup(type -4, 3);	printf("%s [%c]\n", name, atoi(name));

		if (version10p) {
			unsigned char *ptr;
			int bytes;

			for (bytes = 0, ptr = bits; bytes < size; (bytes += 2)) {
				if ((value = next_int (fstream)) < 0)
					RETURN (FALSE);
				*(ptr++) = value;
				if (!padding || ((bytes+2) % bytes_per_line))
					*(ptr++) = value >> 8;
			}
		} else {
			unsigned char *ptr;
			int bytes;

			for (bytes = 0, ptr = bits; bytes < size; bytes++, ptr++) {
				if ((value = next_int (fstream)) < 0) 
					RETURN (FALSE);
				*ptr = value;
				//	printf("value %.3d\t%.2x\t", value, value);
		/*		int j;
				for(j = 0; j < 8; j++){
					printf("%c", (value & (1 << j)) ? '1' : '0' );			// least significant bit first
				//	printf("%d", (value & (0x80 >> j)) ? 1 : 0); 			// right shifting the value will print bits in reverse.
				}
  				if(bytes % bytes_per_line != 0) puts("");
  		*/
			}
		}
	}

	if (!bits)
		RETURN (FALSE);

	*data = bits;
	*width = ww;
	*height = hh;
	if (x_hot)
		*x_hot = hx;
	if (y_hot)
		*y_hot = hy;
	
	return TRUE;
}

#define BITS_IN_BYTE 8

int main(int argc, char **argv)
{
	char *filename = argv[1];
	FILE *f = fopen(filename, "r");
	if(!f) 
		return -1;
	
	uint w, h;
	int x_hot, y_hot;
	uchar *data;
	
	if(!read_bitmap_file_data (f, &w, &h, &data, &x_hot, &y_hot)) {
		printf("Invalid XBM file: %s\n", filename);
		return -1;
	}
	
	// verbose
	if(argv[2]) {
		printf("%dx%d\t(%d %d), data @%p\n", w, h, x_hot, y_hot, &data);
		printf("size %d\n", w * h / BITS_IN_BYTE);
		printf("bytes_per_line %d\n", w / BITS_IN_BYTE);
	}
	
	// dump bits map:
	int i, j, tx = 0;
	
	for(i = 0; i < (w * h / BITS_IN_BYTE); i++) {
		for(j = 0; j < BITS_IN_BYTE; j++){
			printf("%c", (data[i] & (1 << j)) ? '*' : ' ' );		// least significant bit first
		//	printf("%d", (value & (0x80 >> j)) ? 1 : 0); 			// right shifting the value will print bits in reverse.
		}
		tx++;
  		if(tx == (w / BITS_IN_BYTE)) {
  			tx=0;
  			puts("");
		}
	}
	
	return 0;
}