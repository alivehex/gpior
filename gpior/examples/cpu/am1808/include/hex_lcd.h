#ifndef _HEX_LCD_H_
#define _HEX_LCD_H_


#include "types.h"

extern int LCD_open(const char *s);
extern void *LCD_getfb(void);
extern void LCD_enable_backlight(void);
extern void LCD_disable_backlight(void);

#ifndef BLUE
#define BLUE	0x001F
#endif
#ifndef RED
#define RED		0xF800
#endif
#ifndef WHITE	
#define WHITE	0xFFFF
#endif
#ifndef GREEN
#define GREEN	0x07E0
#endif
#ifndef BLACK	
#define BLACK	0x0000
#endif
#ifndef DARK_RED
#define DARK_RED	0x1111
#endif
#ifndef DARK_GREEN
#define DARK_GREEN	0x2222
#endif
#ifndef DARK_BLUE
#define DARK_BLUE	0x4444
#endif
#ifndef LIGHT_GRAY
#define LIGHT_GRAY	0x8888
#endif
#ifndef GRAY
//#define GRAY	0x7777
#endif
#ifndef YELLOW
#define YELLOW	0xBBBB
#endif
#ifndef PINK
#define PINK	0xDDDD
#endif

#define RGB16(r, g, b)	(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

#define GRAY	RGB16(128, 128, 105)
#define DGRAY	RGB16(100, 100, 100)
#define PBLUE	RGB16(65, 105, 225)
#define DBLUE	RGB16(30, 144, 255)


#endif /* ifndef _HEX_LCD_H_ */
