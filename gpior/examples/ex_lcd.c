#include "gpior.h"


#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_ENABLE
#define TRACE_TITLE "LCD"
#include "trace.h"


/*
 * @ LCD example
 */
int LCD_example(void) {
	int i;
	
	TRACE_DEBUG("LCD example");
	LCD_open(20000000);
	LCD_draw_string("!\"#$%&'()*+,-./");
	LCD_draw_string("0123456789:;<=>?@");
	for(i = 0; i < 22; i ++) {
		LCD_draw_symble(i);
	}
	LCD_clear();
	LCD_draw_string("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`");
	LCD_draw_string("abcdefghijklmnopqrstuvwxyz{|}~");
	LCD_clear();
	for(i = 0; i < 27; i ++) {
		LCD_draw_icon(i);
	}

	return 0;
}
