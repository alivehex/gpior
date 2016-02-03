#include "lcd.h"
#include "spi.h"
#include "gpior.h"

#if (GPIOR_LCD5510)

#if (GPIOR_LCD_BITMAP)
extern const uint8 font_5_8[97][5];
#endif

#if (GPIOR_LCD_ICONS)
extern const uint8 icon[][10];
#endif

// SPI write byte
#define LCDSPI_WRITE(dat) 	spi_cs_low(GPIOR_LCD_CH); \
							spi_write_byte(GPIOR_LCD_CH, dat); \
							spi_cs_high(GPIOR_LCD_CH)
// clear the D/C line
#define LCD_SPICMD()		GPIOR_CLRIO(LCD_PORT, LCD_DC_PIN)
// set the D/C line
#define LCD_SPIDAT()		GPIOR_SETIO(LCD_PORT, LCD_DC_PIN)
// clear the RESET line						
#define LCD_RSTL()			GPIOR_CLRIO(LCD_PORT, LCD_RST_PIN)
// set the RESET line	
#define LCD_RSTH()			GPIOR_SETIO(LCD_PORT, LCD_RST_PIN)

// dummy delay
void delay_ms(int time) {
	while(time --);
}

/* @ horizontal addressing (V = 0)
 */
void LCD_draw_bytes(const uint8 *buffer, int nbytes) {
	while(nbytes --) {
		LCDSPI_WRITE(*(buffer ++));
	}
}

/* @ x: [0, 83]
 * @ y: [0, 5] 
 */
void LCD_set_xy(uint8 x, uint8 y) {
	LCD_SPICMD();
	LCDSPI_WRITE(0x40 | y);
	LCDSPI_WRITE(0x80 | x);	
	LCD_SPIDAT();	
}

/*
 * @ clear the LCD
 */
void LCD_clear() {
	int i;
	uint8 buffer = 0x00;
	
	LCD_set_xy(0, 0);
	for(i = 0; i < 504; i ++) {
		LCD_draw_bytes(&buffer, sizeof(buffer));
	}
	LCD_set_xy(0, 0);
}

#if (GPIOR_LCD_BITMAP)
/*
 * @ write char by ascii code 
 */
void LCD_draw_string(uint8 *str) {
	int index;
	uint8 c;
	
	while(*str != 0) {
		c = *str;
		index = c - ' ';
		str ++;
		LCD_draw_bytes(font_5_8[index], 5);
		LCDSPI_WRITE(0x00);
	}
}

/*
 * @ write symble by id
 * @ id = [0, 21]
 */
void LCD_draw_symble(uint8 id) {
	if(id > 21) {
		id = 0;
	}
	LCD_draw_bytes(font_5_8[id + 0x5f], 5);
	LCDSPI_WRITE(0x00);
}
#endif

#if (GPIOR_LCD_ICONS)
/*
 * @ LCD draw icon
 * @ id = [0, 26]
 */
void LCD_draw_icon(uint8 id) {
	LCD_draw_bytes(icon[id], 10);
	LCDSPI_WRITE(0x00);
	LCDSPI_WRITE(0x00);
}
#endif

/*
 * @ LCD IO init
 */
static void LCD_io_init() {
	spi_open(GPIOR_LCD_CH, 1000000);
	GPIOR_CFGIO_OUTPUT(LCD_PORT, LCD_DC_PIN | LCD_RST_PIN);
	LCD_PLAT_INIT();
}

/* Init LCD and IOs
 */
void LCD_open(int spi_baudrate) {
	LCD_io_init();
	// init spi and ios here
	LCD_RSTL();
	delay_ms(1000000);
	LCD_RSTH();
	delay_ms(1000000);
	LCD_SPICMD();
	// config the LCD at 3.3V 
	LCDSPI_WRITE(0x21); // ext cmd
	LCDSPI_WRITE(0xBF); // VLCD
	LCDSPI_WRITE(0x13); // BIAS = 1:48
	LCDSPI_WRITE(0x20); // basic cmd
	LCDSPI_WRITE(0x0c); // normal display
	// clear screen	
	LCD_SPIDAT();
	LCD_clear();
}

#if (0)
// test only
void LCD_test() {
	int i;
	
	TRACE_DEBUG("LCD example");
	LCD_init();
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
}
#endif

#endif // if (GPIOR_LCD5510)
