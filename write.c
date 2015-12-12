#include "common.h"
#include "kernel.h"

#define FB_COMMAND_PORT				0x3D4
#define FB_DATA_PORT					0x3D5
#define FB_HIGH_BYTE_COMMAND	14
#define FB_LOW_BYTE_COMMAND		15

static char *vidptr = (char*)0xb8000; 	//video mem begins here.
unsigned short cursor_pos = 0;

void fb_clear(void)
{

	//unsigned int i = 0;
	unsigned int j = 0;

	/* this loops clears the screen
	* there are 25 lines each of 80 columns; each element takes 2 bytes */
	while(j < 80 * 25 * 2) {
		/* blank character */
		vidptr[j] = ' ';
		/* attribute-byte - light grey on black screen */
		vidptr[j+1] = 0x07;
		j = j + 2;
	}
}

/** fb_write_cell:
 *	Writes a character to position in framebuffer
 * 	TODO: ability to set colors
 *
 * @param i	location in framebuffer
 * @param c the character
 */
 void fb_write_cell(unsigned int i, char c)
 {
	 vidptr[i] = c;
	 vidptr[i+1] = ((0 & 0x0F) << 4) | (2 & 0x0F);	// green on black background
 }

 void fb_write_b(char b)
 {
	 fb_write_cell((cursor_pos*2), b);
	 cursor_pos++;
 }

void fb_move_cursor(unsigned short pos)
{
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT, (unsigned char)((pos >> 8) & 0xFF));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT, (unsigned char) (pos & 0xFF));
}

void fb_write(char *buf)
{
	int i = 0;
	while( buf[i] != '\0')
	{
		fb_write_b(buf[i]);
		i++;
	}
}
