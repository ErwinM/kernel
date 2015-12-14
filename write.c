#include "common.h"
#include "write.h"

unsigned short *vidptr = (unsigned short *)0xB8000; 	//video mem begins here.
uint16_t cursor_x = 0, cursor_y = 0;

void fb_move_cursor();
void fb_scroll(void);
void fb_clear(void)
{
	// Make an attribute byte for the default colours
   uint8_t attributeByte = (0 << 4) | (15 & 0x0F); // white on black
   uint16_t blank = 0x20 | (attributeByte << 8); // space

   int i;
   for (i = 0; i < 80*25; i++)
   {
       vidptr[i] = blank;
   }
	cursor_x = 0;
	cursor_y = 0;
	fb_move_cursor();
}

void fb_put_char(char c)
{
	// Check if we reached the end of the screen and need to scroll
	if ( cursor_y > 25 ) fb_scroll();

	// The background colour is black (0), the foreground is white (15).
	uint8_t backColour = 0;
	uint8_t foreColour = 15;

	// The attribute byte is made up of two nibbles - the lower being the
	// foreground colour, and the upper the background colour.
	uint8_t  attributeByte = (backColour << 4) | (foreColour & 0x0F);

	// The attribute byte is the top 8 bits of the word we have to send to the
	// VGAboard.
	uint16_t attribute = attributeByte << 8;
	uint16_t *where;
	// Handle backspace
	if ( c == 0x08 )
	{
		if ( cursor_x != 0 ) cursor_x--;
	// Handle a newline
	} else if( c == '\n') {
		cursor_x = 0;
	 	cursor_y++;
	} else {
		where = vidptr + (cursor_y * 80 + cursor_x);
		*where = c | attribute;
		if (cursor_x == 80) {
			cursor_x = 0;
			cursor_y++;
		} else {
			cursor_x++;
		}
	}
}

void fb_move_cursor()
{
	uint16_t pos = cursor_y * 80 + cursor_x;
	outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
	outb(FB_DATA_PORT, (pos >> 8));
	outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
	outb(FB_DATA_PORT, pos);
}

void fb_write(char *buf)
{
	int i = 0;
	while( buf[i] != '\0')
	{
		fb_put_char(buf[i]);
		i++;
	}
	fb_move_cursor();
}

void fb_write_dec(uint32_t n)
{
    if (n == 0)
    {
        fb_put_char('0');
        return;
    }

    int32_t acc = n;
    char c[32];
    int i = 0;
    while (acc > 0)
    {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }
    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0)
    {
        c2[i--] = c[j++];
    }
    fb_write(c2);
}

void fb_scroll(void)
{
	// Move last 24 lines up one line
	uint16_t *temp = vidptr + 2*80;
	memcpy( vidptr, temp, 24*80*2 );

	// Clear last line
	uint8_t attributeByte = (0 << 4) | (15 & 0x0F); // white on black
	uint16_t blank = 0x20 | (attributeByte << 8); // space
	for (int i = 0; i <= 80; i++)
	{
			int temp = 24*80 + i;
			vidptr[temp] = blank;
	}
	// Move cursor to correct spot
	cursor_x = 0;
	cursor_y = 24;
	fb_move_cursor();
}
