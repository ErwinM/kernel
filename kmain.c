#include "common.h"

void fb_clear(void);
void fb_write_cell(int i, char c);
void fb_write_b(char b);
void fb_move_cursor(unsigned short pos);
void fb_write(char *buf);
extern short cursor_pos;

void kmain(void)
{
	//const char *str = "erwin's first kernel";
	fb_clear();
	fb_write("Erwin");
	fb_move_cursor(cursor_pos);
}
