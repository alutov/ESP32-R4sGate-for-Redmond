// tft proc

#include "wallpaper.h"          //screen cat
#include "tjpgd.c"              //jpeg decode

// tft define area
#define SPI_PORT    SPI2_HOST

/*-----------------------------------------------------------------------------------/
/ TFT code is my attempt to port some Bodmer tft procedures from arduino to  esp-idf /
/ https://github.com/Bodmer/TFT_eSPI                                                 /
/-----------------------------------------------------------------------------------*/

/***************************************************************************************
**                         Section 5: Font datum enumeration
***************************************************************************************/
//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right
#define L_BASELINE  9 // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

/***************************************************************************************
**                         Section 6: Colour enumeration
***************************************************************************************/
// Default color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_DARKB       0x38E7      /*   0,   0,  15 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_DBLUE       0x0004      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F      
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

uint8_t PIN_NUM_MISO = 25;	//MISO
uint8_t PIN_NUM_MOSI = 23;	//MOSI
uint8_t PIN_NUM_CLK  = 19;	//CLK
uint8_t PIN_NUM_CS   = 16;	// Chip select control pin
uint8_t PIN_NUM_DC   = 17;	// Data / !Command control pin
uint8_t PIN_NUM_RST  = 18;	// Reset pin (could connect to RST pin)
uint8_t PIN_NUM_BCKL = 21;	// TFT_BACKLIGHT
uint8_t PIN_NUM_PWR = 33;	// TFT pwr pin always high

uint8_t blstnum = 0;

int _width = 320;
int _height = 240;


int  cursor_y  = 0;
int  cursor_x  = 0;
int  textfont  = 1;
int  textsize  = 1;
int  textcolor   = 0xFFFF; // White
int  textbgcolor = 0x0008; // Black
int  bitmap_fg = 0xFFFF; // White
int  bitmap_bg = 0x0000; // Black

int  padX = 0;             // No padding
int  isDigits   = false;   // No bounding box adjustment
int  textwrapX  = true;    // Wrap text at end of line when using print stream
int  textwrapY  = false;   // Wrap text at bottom of screen when using print stream
int  textdatum = TL_DATUM; // Top Left text alignment is default
int  fontsloaded = 0;

uint16_t  addr_row = 0xFFFF;
uint16_t  addr_col = 0xFFFF;

bool _swapBytes = false;   // Do not swap colour bytes by default

bool  locked = true;        // Transaction mutex lock flags
bool  inTransaction = false;




/***************************************************************************************
**                         Section 4: Setup fonts
***************************************************************************************/
// Only load the fonts defined in User_Setup.h (to save space)
// Set flag so RLE rendering code is optionally compiled
  #include "Fonts/Font16.h"
  #include "Fonts/Font32rle.h"
  #include "Fonts/Font64rle.h"
  #include "Fonts/Font7srle.h"
  #include "Fonts/Font72rle.h"

// Create a null default font in case some fonts not used (to prevent crash)
const  uint8_t widtbl_null[1] = {0};
const uint8_t chr_null[1] = {0};
const uint8_t* const chrtbl_null[1] = {chr_null};

// This is a structure to conveniently hold information on the default fonts
// Stores pointer to font character image address table, width table and height
typedef struct {
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    uint8_t height;
    uint8_t baseline;
    } fontinfo;

// Now fill the structure
const fontinfo fontdata [] = {
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
   // GLCD font (Font 1) does not have all parameters
   { (const uint8_t *)chrtbl_null, widtbl_null, 8, 7 },

   { (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16},

   // Font 3 current unused
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

   { (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32},

   // Font 5 current unused
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

   { (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64},

   { (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s},

   { (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72}
};


#include "driver/spi_master.h"
#include "driver/gpio.h"



//#define DC_C GPIO.out_w1tc = (1 << PIN_NUM_DC)//;GPIO.out_w1tc = (1 << PIN_NUM_DC)
//#define DC_D GPIO.out_w1ts = (1 << PIN_NUM_DC)//;GPIO.out_w1ts = (1 << PIN_NUM_DC)
#define DC_C gpio_set_level(PIN_NUM_DC, 0);
#define DC_D gpio_set_level(PIN_NUM_DC, 1);




bool     _swapBytes; // Swap the byte order for TFT pushImage()

spi_device_handle_t spi;

//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16

/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;



const lcd_init_cmd_t ili9341_init_cmds[]={
    {0xEF, {0x03, 0x80, 0X02}, 3},

    /* Power contorl B, power control = 0, DC_ENA = 1 */
    {0xCF, {0x00, 0xC1, 0X30}, 3},
    /* Power on sequence control,
     * cp1 keeps 1 frame, 1st frame enable
     * vcl = 0, ddvdh=3, vgh=1, vgl=2
     * DDVDH_ENH=1
     */
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    /* Driver timing control A,
     * non-overlap=default +1
     * EQ=default - 1, CR=default
     * pre-charge=default - 1
     */
    {0xE8, {0x85, 0x00, 0x78}, 3},
    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    /* Pump ratio control, DDVDH=2xVCl */
    {0xF7, {0x20}, 1},
    /* Driver timing control, all=0 unit */
    {0xEA, {0x00, 0x00}, 2},
    /* Power control 1, GVDD=4.75V */
    {0xC0, {0x23}, 1},
    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    {0xC1, {0x10}, 1},
    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    {0xC5, {0x3E, 0x28}, 2},
    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    {0xC7, {0x86}, 1},

//  writedata(TFT_MAD_MX | TFT_MAD_COLOR_ORDER); // Rotation 0 (portrait mode)
//  writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
    {0x36, {0xe8}, 1},

    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Frame rate control, f=fosc, 70Hz fps */
    {0xB1, {0x00, 0x13}, 2},   //100Hz
    /* Enable 3G, disabled */
    {0xF2, {0x00}, 1},
    /* Gamma set, curve 1 */
    {0x26, {0x01}, 1},
    /* Positive gamma correction */
    {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0XF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15},
    /* Negative gamma correction */
    {0xE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15},
    /* Column address set, SC=0, EC=0xEF */
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    /* Page address set, SP=0, EP=0x013F */
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    /* Memory write */
    {0x2C, {0}, 0},
    /* Entry mode set, Low vol detect disabled, normal display */
    {0xB7, {0x07}, 1},
    /* Display function control */
    {0xB6, {0x08, 0x82, 0x27, 0x00}, 4},
    /* Sleep out */
    {0x11, {0}, 0x80},
    /* Display on */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

const lcd_init_cmd_t ili9341_init1_cmds[]={
    {0x36, {0x28}, 1},
};


const lcd_init_cmd_t ili9342_init_cmds[]={
    {0xB9, {0xFF, 0x93, 0x42}, 3},

//    {0xEF, {0x03, 0x80, 0x02}, 3},

    /* Power contorl B, power control = 0, DC_ENA = 1 */
    {0xCF, {0x00, 0xC1, 0x30}, 3},
    /* Power on sequence control,
     * cp1 keeps 1 frame, 1st frame enable
     * vcl = 0, ddvdh=3, vgh=1, vgl=2
     * DDVDH_ENH=1
     */
    {0xED, {0x64, 0x03, 0x12, 0x81}, 4},
    /* Driver timing control A,
     * non-overlap=default +1
     * EQ=default - 1, CR=default
     * pre-charge=default - 1
     */
    {0xE8, {0x85, 0x00, 0x78}, 3},
    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    /* Pump ratio control, DDVDH=2xVCl */
    {0xF7, {0x20}, 1},
    /* Driver timing control, all=0 unit */
    {0xEA, {0x00, 0x00}, 2},
    /* Power control 1, GVDD=4.75V */
    {0xC0, {0x23}, 1},
    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    {0xC1, {0x10}, 1},
    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    {0xC5, {0x3E, 0x28}, 2},
    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    {0xC7, {0x86}, 1},

//  writedata(TFT_MAD_MX | TFT_MAD_COLOR_ORDER); // Rotation 0 (portrait mode)
//  writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
    {0x36, {0x08}, 1},

    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Frame rate control, f=fosc, 70Hz fps */
    {0xB1, {0x00, 0x13}, 2},   //100Hz
    /* Enable 3G, disabled */
    {0xF2, {0x00}, 1},
    /* Gamma set, curve 1 */
    {0x26, {0x01}, 1},
    /* Positive gamma correction */
    {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0XF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15},
    /* Negative gamma correction */
    {0xE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15},
    /* Column address set, SC=0, EC=0xEF */
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    /* Page address set, SP=0, EP=0x013F */
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    /* Memory write */
    {0x2C, {0}, 0},
    /* Entry mode set, Low vol detect disabled, normal display */
    {0xB7, {0x07}, 1},
    /* Display function control */
    {0xB6, {0x08, 0x82, 0x27, 0x00}, 4},
    /* Sleep out */
    {0x11, {0}, 0x80},
    /* Display on */
    {0x29, {0}, 0x80},
    /* Display inversion on */
    {0x21, {0}, 0x0},
    {0, {0}, 0xff},
};

const lcd_init_cmd_t ili9342_init1_cmds[]={
    {0x36, {0xc8}, 1},
};


const lcd_init_cmd_t st7789_init_cmds[]={
    /* Sleep out */
    {0x11, {0}, 0x80},
    /* Normal Display mode on */
    {0x13, {0}, 0x0},
//  writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
    {0x36, {0xe8}, 1},
    /* Pixel format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Porch setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, VGL = -10.43V,  VGH = 13.26V */
    {0xb7, {0x35}, 1},
    /* VCOM Setting, 1.1V */
    {0xbb, {0x28}, 1},
    /* LCM Control */
    {0xc0, {0x00}, 1},
    /* VDV and VRH Command Enable CMDEN = 1, 
	VDV and VRH register value comes from command write */
    {0xc2, {0x01, 0xff}, 2},
    /* VRH Set, VAP(GVDD) = 4.35+( vcom+vcom offset+vdv) V
	VAN(GVCL) = -4.35+( vcom+vcom offset-vdv) V */
    {0xc3, {0x10}, 1},
    /* VDV Set, VDV  = 0V */
    {0xc4, {0x20}, 1},
    /* Frame Rate Control in Normal Mode
	RTNA = 0xf (60Hz)
	NLA  = 0 (dot inversion) */
    {0xc6, {0x0f}, 1},
    /* Power Control 1
	VDS  = 2.3V
	AVCL = -4.8V
	AVDD = 6.8v */
    {0xd0, {0xa4, 0xa1}, 2},
    /* Positive gamma correction */
    {0xE0, {0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x32, 0X44, 0x42, 0x06, 0x0e, 0x12, 0x14, 0x17}, 14},
    /* Negative gamma correction */
    {0xE1, {0xd0, 0x00, 0x02, 0x07, 0x0a, 0x28, 0x31, 0X54, 0x47, 0x0e, 0x1c, 0x17, 0x1b, 0x1e}, 14},
    /* Display inversion off */
    {0x20, {0}, 0},
    /* Display on */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

const lcd_init_cmd_t st7789_init1_cmds[]={
    {0x36, {0x28}, 1},
};


//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
	int dc=(int)t->user;
	gpio_set_level(PIN_NUM_DC, dc);
}


/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length = 8;                   //Command is 8 bits
	t.tx_buffer = &cmd;             //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
}


/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
	esp_err_t ret;
	spi_transaction_t t;
	if (len==0) return;             //no need to send anything
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length = len*8;               //Len is in bytes, transaction length is in bits.
	t.tx_buffer = data;             //Data
	t.user=(void*)1;                //D/C needs to be set to 1
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
}

uint32_t lcd_get_dd(spi_device_handle_t spi, uint8_t cmd, uint8_t len)
{
//write cmd then read up to 4 data bytes
	uint8_t i = len;
	uint32_t rxnum;
	uint32_t rxswnum;
	if (!i) return 0;
	if (i > 3) i = 3;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	if (PIN_NUM_MISO == PIN_NUM_MOSI) {
	t.length = 8;
	t.rxlength = 8*i + 1;
	} else {
	t.length = 8*(i+1);
	t.rxlength = 8*i;
	}
	t.tx_buffer = &cmd;
	t.flags = SPI_TRANS_USE_RXDATA;
	t.user = (void*)0;
	esp_err_t ret = spi_device_polling_transmit(spi, &t);
	assert( ret == ESP_OK );
	rxnum = *(uint32_t*)t.rx_data;
	rxswnum = ((rxnum>>24)&0xff) | ((rxnum<<8)&0xff0000) | ((rxnum>>8)&0xff00) | ((rxnum<<24)&0xff000000);
	if (PIN_NUM_MISO == PIN_NUM_MOSI) {
	rxswnum = rxswnum << 1;
	if (i > 1) rxswnum = rxswnum << 1;
	} else {
	rxswnum = rxswnum << 8;
	if (i > 1) rxswnum = rxswnum << 1;
	}
	rxnum = ((rxswnum>>24)&0xff) | ((rxswnum<<8)&0xff0000) | ((rxswnum>>8)&0xff00) | ((rxswnum<<24)&0xff000000);
	return rxnum;
}


/***************************************************************************************
** Function name:           setSwapBytes
** Description:             Used by 16 bit pushImage() to swap byte order in colours
***************************************************************************************/
void setSwapBytes(bool swap)
{
  _swapBytes = swap;
}

/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void drawPixel(int32_t x, int32_t y, uint32_t color)
{
  // Range checking
  if ((x < 0) || (y < 0) ||(x >= _width) || (y >= _height)) return;
	esp_err_t ret;
	char swdata[4];
	spi_transaction_t t;

//  begin_tft_write();

  // No need to send x if it has not changed (speeds things up)
  if (addr_col != x) {
	swdata[0] = 0x2a;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = (x)>>8 & 0xff;   //Start Col High;
	swdata[1] = (x) & 0xff;      //Start Col Low;
	swdata[2] = 0;//(x1)>>8 & 0xff;   //End Col High;
	swdata[3] = 0;//(x1)& 0xff;      //End Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*4;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	addr_col = x;
	}

  // No need to send y if it has not changed (speeds things up)
  if (addr_row != y) {
	swdata[0] = 0x2b;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = (y)>>8 & 0xff;   //Start Col High;
	swdata[1] = (y) & 0xff;      //Start Col Low;
	swdata[2] = 0;//(y1)>>8 & 0xff;   //End Col High;
	swdata[3] = 0;//(y1)& 0xff;      //End Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*4;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	addr_row = y;
  }
	swdata[0] = 0x2c;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = (color)>>8 & 0xff;   //Start Col High;
	swdata[1] = (color) & 0xff;      //Start Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*2;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
//  end_tft_write();
}




void setWindow(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
	esp_err_t ret;
	char swdata[4];

	addr_row = 0xFFFF;
	addr_col = 0xFFFF;

	swdata[0] = 0x2a;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.

	swdata[0] = (x0)>>8 & 0xff;   //Start Col High;
	swdata[1] = (x0) & 0xff;      //Start Col Low;
	swdata[2] = (x1)>>8 & 0xff;   //End Col High;
	swdata[3] = (x1)& 0xff;      //End Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*4;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = 0x2b;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = (y0)>>8 & 0xff;   //Start Col High;
	swdata[1] = (y0) & 0xff;      //Start Col Low;
	swdata[2] = (y1)>>8 & 0xff;   //End Col High;
	swdata[3] = (y1) & 0xff;      //End Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*4;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = 0x2c;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
}



void tft_Write_16(const uint16_t data)
{
  	uint16_t color16 = (data<<8 | data >>8);
	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=16;                    //transaction length is in bits.
	t.tx_buffer=&color16;               //Data
	t.user=(void*)1;                //D/C needs to be set to 1
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
}

void tft_Write_16S(const uint16_t data)
{
  	uint16_t color16 = data;
	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=16;                    //transaction length is in bits.
	t.tx_buffer=&color16;               //Data
	t.user=(void*)1;                //D/C needs to be set to 1
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
}


void pushPixels(const void* data_in, uint32_t len){
	esp_err_t ret;
	spi_transaction_t t;
	uint8_t buf[64];
  	uint32_t sz = len;
	uint32_t i;
	uint32_t j;
  	uint8_t *bufp = (uint8_t*)data_in;
	while (sz > 0) {
	i = sz;
	if (i > 32) i = 32;
	j = 0;
	if (_swapBytes) {
	while (j < i) {
	buf[2*j+1] = bufp[2*j];
	buf[2*j] = bufp[2*j+1];
	j++;
	}
	} else {
	while (j < i) {
	buf[2*j] = bufp[2*j];
	buf[2*j+1] = bufp[2*j+1];
	j++;
	}
	}
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*2*i;
	t.tx_buffer=buf;
	t.user=(void*)1;                //D/C to 1
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	sz -= i;
	bufp += i*2;
	}
}



void begin_tft_write(){
	gpio_set_level(PIN_NUM_CS, 0);
}
void end_tft_write(){
	gpio_set_level(PIN_NUM_CS, 1);
}





/***************************************************************************************
** Function name:           pushBlock - for ESP32
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void pushBlock(uint16_t color, uint32_t len){
	esp_err_t ret;
	spi_transaction_t t;
  	uint32_t sz = len;
	uint32_t buf[64];
  	uint32_t color32 = (color<<8 | color >>8)<<16 | (color<<8 | color >>8);
	uint32_t i = 0;
	while (i < 16) {
	buf[i] = color32; i++;
	}
	while (sz > 0) {
	i = sz;
	if (i > 32) i = 32;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*2*i;
	t.tx_buffer=&buf;
	t.user=(void*)1;                //D/C to 1
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	sz -= i;
	}
}


/***************************************************************************************
** Function name:           fillRect
** Description:             draw a filled rectangle
***************************************************************************************/
void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  // Clipping
  if ((x >= _width) || (y >= _height)) return;

  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }

  if ((x + w) > _width)  w = _width  - x;
  if ((y + h) > _height) h = _height - y;

  if ((w < 1) || (h < 1)) return;

  setWindow(x, y, x + w - 1, y + h - 1);
  pushBlock(color, w * h);
}



/***************************************************************************************
** Function name:           fillScreen
** Description:             Clear the screen to defined colour
***************************************************************************************/
void fillScreen(uint32_t color)
{
  fillRect(0, 0, _width, _height, color);
}


/***************************************************************************************
** Function name:           width
** Description:             Return the pixel width of display (per current rotation)
***************************************************************************************/
// Return the size of the display (per current rotation)
int16_t width(void)
{
  return _width;
}

/***************************************************************************************
** Function name:           height
** Description:             Return the pixel height of display (per current rotation)
***************************************************************************************/
int16_t height(void)
{
  return _height;
}


/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Line buffer UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t decodeUTF8(uint8_t *buf, uint16_t *index, uint16_t remaining)
{
  uint16_t c = buf[(*index)++];
  //Serial.print("Byte from string = 0x"); Serial.println(c, HEX);

  // 7 bit Unicode
	if ((c & 0x80) == 0x00) return c;
  // 11 bit Unicode
	if (((c & 0xE0) == 0xC0) && (remaining > 1)) {
// À - do 90 ß - do af à - do bo ï - d0 bf ð - d1 80 ÿ - d1 8f
	c = ((c & 0x1F)<<6) | (buf[(*index)++]&0x3F);
// move rus sym to 0x80-0xbf
	if ((c > 0x40f) && (c < 0x450))	return (c - 0x390);
	else if (c == 0xb0) return 0x27;
	else return 0x20;
	}
  // 16 bit Unicode
	if (((c & 0xF0) == 0xE0) && (remaining > 2)) {
	c = ((c & 0x0F)<<12) | ((buf[(*index)++]&0x3F)<<6);
	c = c | ((buf[(*index)++]&0x3F));
// move weather
	if ((c > 0x218f) && (c < 0x219a)) return (c - 0x209a);
	else switch (c) {
	case 0x2600:
	c = 0xf0;	
	break;
	case 0x2601:
	c = 0xf1;	
	break;
	case 0x26C5:
	c = 0xf2;	
	break;
	case 0x26C8:
	c = 0xf3;	
	break;
	case 0x2744:
	c = 0xf4;	
	break;
	default:
//if (fdebug) ESP_LOGI(AP_TAG,"sym=0x%X", c);
	c = 0x20;	
	break;
	}
	return c;
	}

  // 21 bit Unicode not supported so fall-back to extended ASCII
	if ((c & 0xF8) == 0xF0) { 
	uint32_t d = ((c & 0x0F)<<18) | ((buf[(*index)++]&0x3F)<<12);
	d = d | ((buf[(*index)++]&0x3F)<<6);
	d = d | ((buf[(*index)++]&0x3F));
// move weather
	if ((d > 0x1f323) && (d < 0x1f32a)) return (d - 0x1F23A);
	else switch (d) {
/*
	case 0x2600:
	c = 0xf0;	
	break;
*/
	default:
//if (fdebug) ESP_LOGI(AP_TAG,"sym=0x%X", d);
	c = 0x20;	
	break;
	}
	return c;
	}
	return c; // fall-back to extended ASCII
}

uint8_t pgm_read_byte(const uint8_t *cin)
{
uint8_t t = *cin;
return t;
}

uint32_t pgm_read_dword(const uint8_t* const *cin)
{
uint32_t t = (uint32_t)*cin;
return t;
}

/***************************************************************************************
** Function name:           setBitmapColor
** Description:             Set the foreground foreground and background colour
***************************************************************************************/
void setBitmapColor(uint16_t c, uint16_t b)
{
  if (c == b) b = ~c;
  bitmap_fg = c;
  bitmap_bg = b;
}


/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground and background colour
***************************************************************************************/
void setTextColor(uint16_t c, uint16_t b)
{
  textcolor   = c;
  textbgcolor = b;
}

/***************************************************************************************
** Function name:           setTextWrap
** Description:             Define if text should wrap at end of line
***************************************************************************************/
void setTextWrap(bool wrapX, bool wrapY)
{
  textwrapX = wrapX;
  textwrapY = wrapY;
}


/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void setTextDatum(uint8_t d)
{
  textdatum = d;
}

/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t textWidth(const char *string, uint8_t font)
{
  int32_t str_width = 0;
  uint16_t uniCode  = 0;


  if (font>1 && font<9) {
    char *widthtable = (char *)fontdata[font].widthtbl - 32; //subtract the 32 outside the loop
//    char *widthtable = (char *)pgm_read_dword( &(fontdata[font].widthtbl ) ) - 32; //subtract the 32 outside the loop

    while (*string) {
      uniCode = *(string++);
      if (uniCode > 31 && uniCode < 128)
//      str_width += pgm_read_byte( widthtable + uniCode); // Normally we need to subtract 32 from uniCode
//      else str_width += pgm_read_byte( widthtable + 32); // Set illegal character = space width
      str_width += *( widthtable + uniCode); // Normally we need to subtract 32 from uniCode
      else str_width += *(widthtable + 32); // Set illegal character = space width
    }

  }
  else {
      while (*string++) str_width += 6;
  }
  isDigits = false;
  return str_width * textsize;
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit colour sprite or image onto TFT
***************************************************************************************/
void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data)
{

  if ((x >= _width) || (y >= _height)) return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0) { dw += x; dx = -x; x = 0; }
  if (y < 0) { dh += y; dy = -y; y = 0; }

  if ((x + dw) > _width ) dw = _width  - x;
  if ((y + dh) > _height) dh = _height - y;

  if (dw < 1 || dh < 1) return;

//  begin_tft_write();
//  inTransaction = true;

  setWindow(x, y, x + dw - 1, y + dh - 1);

  data += dx + dy * w;

  // Check if whole image can be pushed
  if (dw == w) pushPixels(data, dw * dh);
  else {
    // Push line segments to crop image
    while (dh--)
    {
      pushPixels(data, dw);
      data += w;
    }
  }

//  inTransaction = false;
  end_tft_write();

}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode glyph onto the screen
***************************************************************************************/
  // Any UTF-8 decoding must be done before calling drawChar()
int16_t drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font)
{

  if (font == 1) {
    return 0;
  }

  if ((font == 2) && ((uniCode < 32) || (uniCode > 255))) return 0;
  else if ((font == 4) && ((uniCode < 32) || (uniCode > 255))) return 0;
  else if ((font > 4) && (font < 9) && ((uniCode < 32) || (uniCode > 127))) return 0;

  int32_t width  = 0;
  int32_t height = 0;
  uint32_t flash_address = 0;
  uniCode -= 32;

  if (font == 2) {
    flash_address = pgm_read_dword(&chrtbl_f16[uniCode]);
//    flash_address = chrtbl_f16[uniCode];
    width = *(widtbl_f16 + uniCode);
//    flash_address = pgm_read_dword(&chrtbl_f16[uniCode]);
//    width = pgm_read_byte(widtbl_f16 + uniCode);
    height = chr_hgt_f16;
  }
   if ((font > 2) && (font < 9)) {
      flash_address = pgm_read_dword( (const void*)(pgm_read_dword( &(fontdata[font].chartbl ) ) + uniCode*sizeof(void *)) );
//      width = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[font].widthtbl ) ) + uniCode );
//      height= pgm_read_byte( &fontdata[font].height );
      width = *(fontdata[font].widthtbl + uniCode );
      height = fontdata[font].height;

    }

  int32_t w = width;
  int32_t pX      = 0;
  int32_t pY      = y;
  uint8_t line = 0;

    if (x + width * textsize > (int16_t)_width) return width * textsize ;
    if (y - 4 + height * textsize > (int16_t)_height) return width * textsize ;
  if (font == 2) {
    w = w + 6; // Should be + 7 but we need to compensate for width increment
    w = w / 8;

    if (textcolor == textbgcolor || textsize != 1) {
      //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
//      inTransaction = true;

      for (int32_t i = 0; i < height; i++) {
        if (textcolor != textbgcolor) fillRect(x, pY, width * textsize, textsize, textbgcolor);

        for (int32_t k = 0; k < w; k++) {
//          line = pgm_read_byte((uint8_t *)flash_address + w * i + k);
          line = *((uint8_t *)flash_address + w * i + k);
          if (line) {
            if (textsize == 1) {
              pX = x + k * 8;
              if (line & 0x80) drawPixel(pX, pY, textcolor);
              if (line & 0x40) drawPixel(pX + 1, pY, textcolor);
              if (line & 0x20) drawPixel(pX + 2, pY, textcolor);
              if (line & 0x10) drawPixel(pX + 3, pY, textcolor);
              if (line & 0x08) drawPixel(pX + 4, pY, textcolor);
              if (line & 0x04) drawPixel(pX + 5, pY, textcolor);
              if (line & 0x02) drawPixel(pX + 6, pY, textcolor);
              if (line & 0x01) drawPixel(pX + 7, pY, textcolor);
            }
            else {
              pX = x + k * 8 * textsize;
              if (line & 0x80) fillRect(pX, pY, textsize, textsize, textcolor);
              if (line & 0x40) fillRect(pX + textsize, pY, textsize, textsize, textcolor);
              if (line & 0x20) fillRect(pX + 2 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x10) fillRect(pX + 3 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x08) fillRect(pX + 4 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x04) fillRect(pX + 5 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x02) fillRect(pX + 6 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x01) fillRect(pX + 7 * textsize, pY, textsize, textsize, textcolor);
            }
          }
        }
        pY += textsize;
      }

//      inTransaction = false;
      end_tft_write();
    }
    else { // Faster drawing of characters and background using block write
//      begin_tft_write();

      setWindow(x, y, x + width - 1, y + height - 1);

      uint8_t mask;
      for (int32_t i = 0; i < height; i++) {
        pX = width;
        for (int32_t k = 0; k < w; k++) {
//          line = pgm_read_byte((uint8_t *) (flash_address + w * i + k) );
          line = *((uint8_t *) (flash_address + w * i + k) );
          mask = 0x80;
          while (mask && pX) {
            if (line & mask) {tft_Write_16(textcolor);}
            else {tft_Write_16(textbgcolor);}
            pX--;
            mask = mask >> 1;
          }
        }
        if (pX) {tft_Write_16(textbgcolor);}
      }

      end_tft_write();
    }
  } else

  // Font is not 2 and hence is RLE encoded
  {
//    begin_tft_write();
//    inTransaction = true;

    w *= height; // Now w is total number of pixels in the character
    if ((textsize != 1) || (textcolor == textbgcolor)) {
      if (textcolor != textbgcolor) fillRect(x, pY, width * textsize, textsize * height, textbgcolor);
      int32_t px = 0, py = pY; // To hold character block start and end column and row values
      int32_t pc = 0; // Pixel count
      uint8_t np = textsize * textsize; // Number of pixels in a drawn pixel

      uint8_t tnp = 0; // Temporary copy of np for while loop
      uint8_t ts = textsize - 1; // Temporary copy of textsize
      // 16 bit pixel count so maximum font size is equivalent to 180x180 pixels in area
      // w is total number of pixels to plot to fill character block
      while (pc < w) {
//        line = pgm_read_byte((uint8_t *)flash_address);
        line = *((uint8_t *)flash_address);
        flash_address++;
        if (line & 0x80) {
          line &= 0x7F;
          line++;
          if (ts) {
            px = x + textsize * (pc % width); // Keep these px and py calculations outside the loop as they are slow
            py = y + textsize * (pc / width);
          }
          else {
            px = x + pc % width; // Keep these px and py calculations outside the loop as they are slow
            py = y + pc / width;
          }
          while (line--) { // In this case the while(line--) is faster
            pc++; // This is faster than putting pc+=line before while()?
            setWindow(px, py, px + ts, py + ts);

            if (ts) {
              tnp = np;
              while (tnp--) {tft_Write_16(textcolor);}
            }
            else {tft_Write_16(textcolor);}
            px += textsize;

            if (px >= (x + width * textsize)) {
              px = x;
              py += textsize;
            }
          }
        }
        else {
          line++;
          pc += line;
        }
      }
    }
    else { // Text colour != background && textsize = 1
           // so use faster drawing of characters and background using block write
      setWindow(x, y, x + width - 1, y + height - 1);

      // Maximum font size is equivalent to 180x180 pixels in area
      while (w > 0) {
//        line = pgm_read_byte((uint8_t *)flash_address++); // 8 bytes smaller when incrementing here
        line = *((uint8_t *)flash_address++); // 8 bytes smaller when incrementing here
        if (line & 0x80) {
          line &= 0x7F;
          line++; w -= line;
          pushBlock(textcolor,line);
        }
        else {
          line++; w -= line;
          pushBlock(textbgcolor,line);
        }
      }
    }
//    inTransaction = false;
    end_tft_write();
  }
  // End of RLE font rendering
  return width * textsize;    // x +
}







/***************************************************************************************
** Function name:           drawString (with or without user defined font)
** Description :            draw string with padding if it is defined
***************************************************************************************/
// With font number. Note: font number is over-ridden if a smooth font is loaded
int16_t drawString(const char *string, int32_t poX, int32_t poY, uint8_t font)
{
  int16_t sumX = 0;
  uint8_t padding = 1, baseline = 0;
  uint16_t cwidth = textWidth(string, font); // Find the pixel width of the string in the font
  uint16_t cheight = 8 * textsize;
  uint8_t stfont = font;

  if (font != 1) {
    baseline = fontdata[font].baseline * textsize;
    cheight = fontdata[font].height * textsize;
  }

  if (textdatum || padX) {

    switch(textdatum) {
      case TC_DATUM:
        poX -= cwidth/2;
        padding += 1;
        break;
      case TR_DATUM:
        poX -= cwidth;
        padding += 2;
        break;
      case ML_DATUM:
        poY -= cheight/2;
        //padding += 0;
        break;
      case MC_DATUM:
        poX -= cwidth/2;
        poY -= cheight/2;
        padding += 1;
        break;
      case MR_DATUM:
        poX -= cwidth;
        poY -= cheight/2;
        padding += 2;
        break;
      case BL_DATUM:
        poY -= cheight;
        //padding += 0;
        break;
      case BC_DATUM:
        poX -= cwidth/2;
        poY -= cheight;
        padding += 1;
        break;
      case BR_DATUM:
        poX -= cwidth;
        poY -= cheight;
        padding += 2;
        break;
      case L_BASELINE:
        poY -= baseline;
        //padding += 0;
        break;
      case C_BASELINE:
        poX -= cwidth/2;
        poY -= baseline;
        padding += 1;
        break;
      case R_BASELINE:
        poX -= cwidth;
        poY -= baseline;
        padding += 2;
        break;
    }
    // Check coordinates are OK, adjust if not
    if (poX < 0) poX = 0;
    if (poX+cwidth > width())   poX = width() - cwidth;
    if (poY < 0) poY = 0;
    if (poY+cheight-baseline> height()) poY = height() - cheight;
  }


//  int8_t xo = 0;

  uint16_t len = strlen(string);
  uint16_t n = 0;

  {
    while (n < len) {
	uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
	if ((uniCode == 0x5c) && len) {	
	uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
	switch (uniCode) {
	case 0x6e:
   	sumX = 0;
	poX = 0;
	if (stfont == 2) poY = poY + 16;
	else if (stfont == 4) poY = poY + 26;
	break;
	case 0x5c:
   	sumX = 0;
	poX = 0;
	if (stfont == 2) poY = poY + 16;
	else if (stfont == 4) poY = poY + 26;
	break;
	case 0x46:
	if ((sumX) && (stfont == 2)) {
   	sumX = 0;
	poX = 0;
	poY = poY + 16;
	}
	stfont = 4;
	break;
	case 0x66:
	if ((sumX) && (stfont == 4)) {
   	sumX = 0;
	poX = 0;
	poY = poY + 26;
	}
	stfont = 2;
	break;

	case 0x30:
	setTextColor(TFT_WHITE, TFT_BLACK);
	break;
	case 0x31:
	setTextColor(TFT_BLUE, TFT_BLACK);
	break;
	case 0x32:
	setTextColor(TFT_GREEN, TFT_BLACK);
	break;
	case 0x33:
	setTextColor(TFT_RED, TFT_BLACK);
	break;
	case 0x34:
	setTextColor(TFT_CYAN, TFT_BLACK);
	break;
	case 0x35:
	setTextColor(TFT_MAGENTA, TFT_BLACK);
	break;
	case 0x36:
	setTextColor(TFT_YELLOW, TFT_BLACK);
	break;
	case 0x37:
	setTextColor(TFT_WHITE, TFT_NAVY);
	break;
	case 0x38:
	setTextColor(TFT_WHITE, TFT_DARKGREEN);
	break;
	case 0x39:
	setTextColor(TFT_WHITE, TFT_RED);
	break;

	default:
        sumX += drawChar((int16_t)0x5c, poX+sumX, poY, stfont);
        sumX += drawChar(uniCode, poX+sumX, poY, stfont);
	break;
	}
	} else sumX += drawChar(uniCode, poX+sumX, poY, stfont);
    }
  }

return sumX;
}

/***************************************************************************************
** Function name:           drawHString (with or without user defined font)
** Description :            draw string with padding if it is defined
***************************************************************************************/
// With font number. Note: font number is over-ridden if a smooth font is loaded
int16_t drawHString(const char *string, int32_t poX, int32_t poY, uint8_t font)
{
  int16_t sumX = 0;
  uint8_t padding = 1, baseline = 0;
  uint16_t cwidth = textWidth(string, font); // Find the pixel width of the string in the font
  uint16_t cheight = 8 * textsize;
  uint8_t stfont = font;

  if (font != 1) {
    baseline = fontdata[font].baseline * textsize;
    cheight = fontdata[font].height * textsize;
  }

  if (textdatum || padX) {

    switch(textdatum) {
      case TC_DATUM:
        poX -= cwidth/2;
        padding += 1;
        break;
      case TR_DATUM:
        poX -= cwidth;
        padding += 2;
        break;
      case ML_DATUM:
        poY -= cheight/2;
        //padding += 0;
        break;
      case MC_DATUM:
        poX -= cwidth/2;
        poY -= cheight/2;
        padding += 1;
        break;
      case MR_DATUM:
        poX -= cwidth;
        poY -= cheight/2;
        padding += 2;
        break;
      case BL_DATUM:
        poY -= cheight;
        //padding += 0;
        break;
      case BC_DATUM:
        poX -= cwidth/2;
        poY -= cheight;
        padding += 1;
        break;
      case BR_DATUM:
        poX -= cwidth;
        poY -= cheight;
        padding += 2;
        break;
      case L_BASELINE:
        poY -= baseline;
        //padding += 0;
        break;
      case C_BASELINE:
        poX -= cwidth/2;
        poY -= baseline;
        padding += 1;
        break;
      case R_BASELINE:
        poX -= cwidth;
        poY -= baseline;
        padding += 2;
        break;
    }
    // Check coordinates are OK, adjust if not
    if (poX < 0) poX = 0;
    if (poX+cwidth > width())   poX = width() - cwidth;
    if (poY < 0) poY = 0;
    if (poY+cheight-baseline> height()) poY = height() - cheight;
  }


//  int8_t xo = 0;

  uint16_t len = strlen(string);
  uint16_t n = 0;
  uint8_t  h = 0;
  uint16_t p1 = 0;
  uint16_t p2 = 0;
  uint16_t p3 = 0;

  {
    while (n < len) {
	uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
	p3 = p2;
	p2 = p1;
	p1 = uniCode;
	if ((p3 == 0x3c) && (p2 == 0x62) && (p1 == 0x72)) {
   	sumX = 0;
	poX = 0;
	if (stfont == 2) poY = poY + 16;
	else if (stfont == 4) poY = poY + 26;
	} else if (uniCode == 0x3C) h = 1;
	if(!h) sumX += drawChar(uniCode, poX+sumX, poY, stfont);
        if (uniCode == 0x3E) h = 0;
    }
  }

return sumX;
}




//void  tfstate (const char* string) {
//	setTextColor(TFT_WHITE, TFT_BLACK);
//  	uint32_t sumx = drawString(string, 0, 224, 2);
//	fillRect(sumx,224,320-sumx,16,TFT_BLACK);
//	setTextColor(TFT_WHITE, TFT_BLACK);
//}



//Initialize the display
uint8_t lcd_init(spi_device_handle_t spi)
{
	uint8_t result = 0;
	int  cmd = 0;
	uint32_t lcd_id;

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,   // timer mode
        .timer_num = LEDC_TIMER_0,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };

    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = PIN_NUM_BCKL,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
    };

//Initialize non-SPI GPIOs
	if (PIN_NUM_DC > (MxPOutP -1)) return result;
	gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
	mygp_iomux_out(PIN_NUM_DC);
	if (PIN_NUM_PWR) {
	if (PIN_NUM_PWR < MxPOutP) {
	gpio_set_direction(PIN_NUM_PWR, GPIO_MODE_OUTPUT);
	mygp_iomux_out(PIN_NUM_PWR);
	gpio_set_level(PIN_NUM_PWR, 1);
	} else i2c_axpin_set (&f_i2cdev, PIN_NUM_PWR, 255);
	}
	vTaskDelay(200 / portTICK_RATE_MS);
	bStateS = 0;
//Init backlight
	if (PIN_NUM_BCKL) {	
	if (PIN_NUM_BCKL < MxPOutP) {	
	ledc_timer_config(&ledc_timer);
        ledc_channel_config(&ledc_channel);
	ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS);
	ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
	} else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, bStateS);
	}
//Reset the display
	if (PIN_NUM_RST) {
	if (PIN_NUM_BCKL < MxPOutP) {	
	gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
	mygp_iomux_out(PIN_NUM_RST);
	gpio_set_level(PIN_NUM_RST, 1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(PIN_NUM_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(PIN_NUM_RST, 1);
	vTaskDelay(200 / portTICK_RATE_MS);
	} else {
	i2c_axpin_set (&f_i2cdev, PIN_NUM_RST, 255);
	vTaskDelay(100 / portTICK_RATE_MS);
	i2c_axpin_set (&f_i2cdev, PIN_NUM_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	i2c_axpin_set (&f_i2cdev, PIN_NUM_RST, 255);
	vTaskDelay(200 / portTICK_RATE_MS);
	}
	} else vTaskDelay(700 / portTICK_RATE_MS);
    //detect LCD type
	lcd_id = lcd_get_dd(spi, 0x04, 3);
	if (fdebug) ESP_LOGI(AP_TAG,"Display identification information (0x04h) = %x", lcd_id);
	if (lcd_id == 0x0000) {         //0x0, ili9341
    //Send all the commands
	while (ili9341_init_cmds[cmd].databytes!=0xff) {
	lcd_cmd(spi, ili9341_init_cmds[cmd].cmd);
	lcd_data(spi, ili9341_init_cmds[cmd].data, ili9341_init_cmds[cmd].databytes&0x1F);
	if (ili9341_init_cmds[cmd].databytes&0x80) {
	vTaskDelay(100 / portTICK_RATE_MS);
	}
	cmd++;
	}
    //Send flip command
	if (tft_flip) {
	lcd_cmd(spi, ili9341_init1_cmds[0].cmd);
	lcd_data(spi, ili9341_init1_cmds[0].data, ili9341_init1_cmds[0].databytes&0x1F);
	}
	vTaskDelay(100 / portTICK_RATE_MS);
	lcd_id = lcd_get_dd(spi, 0x0a, 1);
	if (fdebug) ESP_LOGI(AP_TAG,"Display Power Mode (0x0ah) = %x", lcd_id);
	if ((lcd_id & 0x08) == 0x08) {
	///Enable backlight
	fillScreen(0);
        if (fdebug) ESP_LOGI(AP_TAG, "ILI9341 LCD detected.");
	bStateS = 255;
	if (PIN_NUM_BCKL) {
	if (PIN_NUM_BCKL < MxPOutP) {	
	if ((f_i2cdev & 0x40000000) && !(pwr_batmode & 0x06)) ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS >>= 4);
	else ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS);
	ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
	} else {
	if ((f_i2cdev & 0x20000000) && !(pwr_batmode & 0x06)) {
	if (bStateS) i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, (bStateS >> 2) | 1);
	else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, 0);
	} else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, bStateS); 
	}
	}
	result = 1;
	} else if (fdebug) ESP_LOGI(AP_TAG, "ILI9341 LCD init error.");

	} else if (lcd_id == 0xe3) {  //0x80f1, ili9342
    //Send all the commands
	while (ili9342_init_cmds[cmd].databytes!=0xff) {
	lcd_cmd(spi, ili9342_init_cmds[cmd].cmd);
	lcd_data(spi, ili9342_init_cmds[cmd].data, ili9342_init_cmds[cmd].databytes&0x1F);
	if (ili9342_init_cmds[cmd].databytes&0x80) {
	vTaskDelay(100 / portTICK_RATE_MS);
	}
	cmd++;
	}
    //Send flip command
	if (tft_flip) {
	lcd_cmd(spi, ili9342_init1_cmds[0].cmd);
	lcd_data(spi, ili9342_init1_cmds[0].data, ili9342_init1_cmds[0].databytes&0x1F);
	}
	vTaskDelay(100 / portTICK_RATE_MS);
	lcd_id = lcd_get_dd(spi, 0x0a, 1);
	if (fdebug) ESP_LOGI(AP_TAG,"Display Power Mode (0x0ah) = %x", lcd_id);
	if ((lcd_id & 0x08) == 0x08) {
	///Enable backlight
	fillScreen(0);
        if (fdebug) ESP_LOGI(AP_TAG, "ILI9342 LCD detected.");
	bStateS = 255;
	if (PIN_NUM_BCKL) {
	if (PIN_NUM_BCKL < MxPOutP) {	
	if ((f_i2cdev & 0x40000000) && !(pwr_batmode & 0x06)) ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS >>= 4);
	else ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS);
	ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
	} else {
	if ((f_i2cdev & 0x20000000) && !(pwr_batmode & 0x06)) {
	if (bStateS) i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, (bStateS >> 2) | 1);
	else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, 0);
	} else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, bStateS); 
	}
	}
	result = 2;
	} else if (fdebug) ESP_LOGI(AP_TAG, "ILI9342 LCD init error.");

	} else if (lcd_id == 0x848580) {  //??? 0x4000c0, st7789
    //Send all the commands
	while (st7789_init_cmds[cmd].databytes!=0xff) {
	lcd_cmd(spi, st7789_init_cmds[cmd].cmd);
	lcd_data(spi, st7789_init_cmds[cmd].data, st7789_init_cmds[cmd].databytes&0x1F);
	if (st7789_init_cmds[cmd].databytes&0x80) {
	vTaskDelay(100 / portTICK_RATE_MS);
	}
	cmd++;
	}
    //Send flip command
	if (tft_flip) {
	lcd_cmd(spi, st7789_init1_cmds[0].cmd);
	lcd_data(spi, st7789_init1_cmds[0].data, st7789_init1_cmds[0].databytes&0x1F);
	}
	vTaskDelay(100 / portTICK_RATE_MS);
	lcd_id = lcd_get_dd(spi, 0x0a, 1);
	if (fdebug) ESP_LOGI(AP_TAG,"Display Power Mode (0x0ah) = %x", lcd_id);
	if ((lcd_id & 0x08) == 0x08) {
	///Enable backlight
	fillScreen(0);
        if (fdebug) ESP_LOGI(AP_TAG, "ST7789 LCD detected.");
	bStateS = 255;
	if (PIN_NUM_BCKL) {
	if (PIN_NUM_BCKL < MxPOutP) {	
	if ((f_i2cdev & 0x40000000) && !(pwr_batmode & 0x06)) ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS >>= 4);
	else ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, bStateS);
	ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
	} else {
	if ((f_i2cdev & 0x20000000) && !(pwr_batmode & 0x06)) {
	if (bStateS) i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, (bStateS >> 2) | 1);
	else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, 0);
	} else i2c_axpin_set (&f_i2cdev, PIN_NUM_BCKL, bStateS); 
	}
	}
	result = 3;
	} else if (fdebug) ESP_LOGI(AP_TAG, "ST7789 LCD init error.");
	} else {
        if (fdebug) ESP_LOGI(AP_TAG, "LCD not found.");
	}
	return result;
}



uint8_t tftinit()
{
	uint8_t result = 0;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	if ((PIN_NUM_MOSI > 21) || (PIN_NUM_MISO > 21) || (PIN_NUM_CLK > 21) || (PIN_NUM_CS > 21)) return result;
#else
	if ((PIN_NUM_MOSI > 33) || (PIN_NUM_MISO > 39) || (PIN_NUM_CLK > 33) || (PIN_NUM_CS > 33)) return result;
#endif
	esp_err_t ret;
	spi_bus_config_t buscfg = {
	.miso_io_num=PIN_NUM_MISO,
	.mosi_io_num=PIN_NUM_MOSI,
	.sclk_io_num=PIN_NUM_CLK,
	.quadwp_io_num = -1,
	.quadhd_io_num = -1,
	};
	spi_device_interface_config_t devcfg={
	.clock_speed_hz=20*1000*1000,           //Clock out at 20 MHz
	.mode = 0,                                //SPI mode 0
	.spics_io_num = PIN_NUM_CS,             //CS pin
	.queue_size = 7,                          //We want to be able to queue 7 transactions at a time
	.pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
	.command_bits = 0,
	.address_bits = 0,
	.dummy_bits = 0,
	.duty_cycle_pos = 0,
	.cs_ena_pretrans = 0,
	.cs_ena_posttrans = 0,
	.input_delay_ns = 0,
	.flags = 0,
	.post_cb = 0
	};

	if (PIN_NUM_MISO == PIN_NUM_MOSI) {
	buscfg.miso_io_num=-1;
    	devcfg.flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE;
	}

//Initialize the SPI bus
	if (PIN_NUM_MISO == PIN_NUM_MOSI) ret=spi_bus_initialize(SPI_PORT, &buscfg, 0);
	else ret=spi_bus_initialize(SPI_PORT, &buscfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);
//Attach the LCD to the SPI bus
	ret=spi_bus_add_device(SPI_PORT, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);
//Initialize the LCD
	result = lcd_init(spi);
	if (result) {
	char buf[8];
	fillRect(0,0,320,52,TFT_BLACK);
	setTextColor(TFT_WHITE, TFT_BLACK);
  	uint32_t sumx = 0;
#ifdef CONFIG_IDF_TARGET_ESP32C3
	itoa(CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ,buf,10);
	sumx = drawString("esp32c3/", 0, 0, 4);
#else
	itoa(CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,buf,10);
	sumx = drawString("esp32/", 0, 0, 4);
#endif
	sumx += drawString(buf, sumx, 0, 4);
	sumx += drawString(" gate", sumx, 0, 4);
	sumx = drawString("ver:  ", 0, 26, 4);
	setTextColor(TFT_YELLOW, TFT_BLACK);
	sumx += drawString(AP_VER, sumx, 26, 4);
	fillRect(sumx,224,320-sumx,16,TFT_BLACK);
	setSwapBytes(true);
	pushImage(0, 52, 320, 240, wallpaper);
	}
	return result;
}

void tftclock()
{
  	uint32_t sumx = 0;
	time_t now;
	char stime[16];
	char sdate[16];
	char sday[8];
	char buff[8];
	int  tmp;
	stime[0] = 0;	
	sdate[0] = 0;	
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
       	itoa(timeinfo.tm_hour,buff,10);
       	if(timeinfo.tm_hour < 10) strcat(stime,"0");
	strcat(stime,buff);
	strcat(stime,":");
       	itoa(timeinfo.tm_min,buff,10);
       	if(timeinfo.tm_min < 10) strcat(stime,"0");
	strcat(stime,buff);
	strcat(stime,":");
       	itoa(timeinfo.tm_sec,buff,10);
       	if(timeinfo.tm_sec < 10) strcat(stime,"0");
	strcat(stime,buff);
	tmp = timeinfo.tm_year;
	while (tmp > 100) tmp = tmp - 100;
	itoa(tmp,buff,10);
       	if(tmp < 10) strcat(sdate,"0");
	strcat(sdate,buff);
	strcat(sdate,"/");
	tmp = timeinfo.tm_mon + 1;
	itoa(tmp,buff,10);
       	if(tmp < 10) strcat(sdate,"0");
	strcat(sdate,buff);
	strcat(sdate,"/");
	itoa(timeinfo.tm_mday,buff,10);
	if(timeinfo.tm_mday < 10) strcat(sdate,"0");
	strcat(sdate,buff);
	switch (timeinfo.tm_wday) {
	case 0:
	strcpy(sday,"Su");
	break;
	case 1:
	strcpy(sday,"M");
	break;
	case 2:
	strcpy(sday,"Tu");
	break;
	case 3:
	strcpy(sday,"W");
	break;
	case 4:
	strcpy(sday,"Th");
	break;
	case 5:
	strcpy(sday,"Fr");
	break;
	case 6:
	strcpy(sday,"Sa");
	break;
	}
	setTextColor(TFT_WHITE, TFT_BLACK);
	drawString(stime,0,0,7);
	setTextColor(TFT_GREEN, TFT_BLACK);
	sumx = 220;
        if (timeinfo.tm_year > 100) sumx += drawString(sday,sumx,26,4);
	fillRect(sumx, 26,250-sumx,26,TFT_BLACK);
	sumx = 220;
        if (timeinfo.tm_year > 100) sumx += drawString(sdate,sumx,0,4);
	setTextColor(TFT_WHITE, TFT_BLACK);
}

void blstnum_inc() {
	if (blstnum == 253) return;
	else if (f_update) blstnum = 254;
	else if (BleDevStA.REQ_NAME[0] || BleDevStB.REQ_NAME[0] || BleDevStC.REQ_NAME[0] || BleDevStD.REQ_NAME[0] || BleDevStE.REQ_NAME[0]) {
	int i = 0;
	while (i < 5) {
	blstnum++;
	if (blstnum > 4) blstnum = 0;
	if ((blstnum == 0) && BleDevStA.REQ_NAME[0]) i = 5;
	if ((blstnum == 1) && BleDevStB.REQ_NAME[0]) i = 5;
	if ((blstnum == 2) && BleDevStC.REQ_NAME[0]) i = 5;
	if ((blstnum == 3) && BleDevStD.REQ_NAME[0]) i = 5;
	if ((blstnum == 4) && BleDevStE.REQ_NAME[0]) i = 5;
	i++;
	}
	} else blstnum = 255;
}
void tfststr(char* in1, char* in2, char* in3)
{
	if (in1 == NULL) return;
  	uint32_t sumx = 0;
	setTextColor(TFT_WHITE, TFT_BLACK);
	sumx += drawString(in1, sumx, 224, 2);
	if (in2 != NULL) {
	setTextColor(TFT_YELLOW, TFT_BLACK);
	sumx += drawString(in2, sumx, 224, 2);
	if (in3 != NULL) {
	setTextColor(TFT_WHITE, TFT_BLACK);
	sumx += drawString(in3, sumx, 224, 2);
	}
	}
	fillRect(sumx,224,320-sumx,16,TFT_BLACK);
	setTextColor(TFT_GREEN, TFT_BLACK);
}

void tfblestate(uint8_t tmr)
{
	uint8_t blstnum1 = blstnum + 1;
        struct BleDevSt *ptr;
	switch (blstnum) {
	case 1:
	ptr = &BleDevStB;
	break;
	case 2:
	ptr = &BleDevStC;
	break;
	case 3:
	ptr = &BleDevStD;
	break;
	case 4:
	ptr = &BleDevStE;
	break;
	default:
	ptr = &BleDevStA;
	break;
	}
	char buf[64];  
	char buff[16];
  	uint32_t sumx = 0;
	uint32_t sumy = 0;
	uint32_t sumz = 0;
	int  mpos = 214;
	sumx = 250;
	uint16_t sym;
	uint16_t bkg;
	if (blstnum == 0) bkg = TFT_DARKB;
	else bkg = TFT_BLACK;
	if (!BleDevStA.btauthoriz) sym = TFT_DARKGREY;
	else if (!BleDevStA.bState && BleDevStA.bHeat) sym = TFT_YELLOW;
	else if (!BleDevStA.bState) sym = TFT_BLUE;
	else if (BleDevStA.bState == 254) sym = TFT_DARKGREY;
	else if ((BleDevStA.DEV_TYP > 15) && (BleDevStA.DEV_TYP < 24) && (BleDevStA.bState == 1)) sym = TFT_WHITE;
	else if ((BleDevStA.DEV_TYP > 15) && (BleDevStA.DEV_TYP < 17) && (BleDevStA.bState == 5)) sym = TFT_YELLOW;
	else if ((BleDevStA.DEV_TYP > 16) && (BleDevStA.DEV_TYP < 24) && (BleDevStA.bState == 2)) sym = TFT_YELLOW;
	else sym = TFT_RED;
	setTextColor(sym, bkg);
        sumx += drawString("1",sumx,26,4);

	if (blstnum == 1) bkg = TFT_DARKB;
	else bkg = TFT_BLACK;
	if (!BleDevStB.btauthoriz) sym = TFT_DARKGREY;
	else if (!BleDevStB.bState && BleDevStB.bHeat) sym = TFT_YELLOW;
	else if (!BleDevStB.bState) sym = TFT_BLUE;
	else if (BleDevStB.bState == 254) sym = TFT_DARKGREY;
	else if ((BleDevStB.DEV_TYP > 15) && (BleDevStB.DEV_TYP < 24) && (BleDevStB.bState == 1)) sym = TFT_WHITE;
	else if ((BleDevStB.DEV_TYP > 15) && (BleDevStB.DEV_TYP < 17) && (BleDevStB.bState == 5)) sym = TFT_YELLOW;
	else if ((BleDevStB.DEV_TYP > 16) && (BleDevStB.DEV_TYP < 24) && (BleDevStB.bState == 2)) sym = TFT_YELLOW;
	else sym = TFT_RED;
	setTextColor(sym, bkg);
        sumx += drawString("2",sumx,26,4);

	if (blstnum == 2) bkg = TFT_DARKB;
	else bkg = TFT_BLACK;
	if (!BleDevStC.btauthoriz) sym = TFT_DARKGREY;
	else if (!BleDevStC.bState && BleDevStC.bHeat) sym = TFT_YELLOW;
	else if (!BleDevStC.bState) sym = TFT_BLUE;
	else if (BleDevStC.bState == 254) sym = TFT_DARKGREY;
	else if ((BleDevStC.DEV_TYP > 15) && (BleDevStC.DEV_TYP < 24) && (BleDevStC.bState == 1)) sym = TFT_WHITE;
	else if ((BleDevStC.DEV_TYP > 15) && (BleDevStC.DEV_TYP < 17) && (BleDevStC.bState == 5)) sym = TFT_YELLOW;
	else if ((BleDevStC.DEV_TYP > 16) && (BleDevStC.DEV_TYP < 24) && (BleDevStC.bState == 2)) sym = TFT_YELLOW;
	else sym = TFT_RED;
	setTextColor(sym, bkg);
        sumx += drawString("3",sumx,26,4);

	if (blstnum == 3) bkg = TFT_DARKB;
	else bkg = TFT_BLACK;
	if (!BleDevStD.btauthoriz) sym = TFT_DARKGREY;
	else if (!BleDevStD.bState && BleDevStD.bHeat) sym = TFT_YELLOW;
	else if (!BleDevStD.bState) sym = TFT_BLUE;
	else if (BleDevStD.bState == 254) sym = TFT_DARKGREY;
	else if ((BleDevStD.DEV_TYP > 15) && (BleDevStD.DEV_TYP < 24) && (BleDevStD.bState == 1)) sym = TFT_WHITE;
	else if ((BleDevStD.DEV_TYP > 15) && (BleDevStD.DEV_TYP < 17) && (BleDevStD.bState == 5)) sym = TFT_YELLOW;
	else if ((BleDevStD.DEV_TYP > 16) && (BleDevStD.DEV_TYP < 24) && (BleDevStD.bState == 2)) sym = TFT_YELLOW;
	else sym = TFT_RED;
	setTextColor(sym, bkg);
        sumx += drawString("4",sumx,26,4);

	if (blstnum == 4) bkg = TFT_DARKB;
	else bkg = TFT_BLACK;
	if (!BleDevStE.btauthoriz) sym = TFT_DARKGREY;
	else if (!BleDevStE.bState && BleDevStE.bHeat) sym = TFT_YELLOW;
	else if (!BleDevStE.bState) sym = TFT_BLUE;
	else if (BleDevStE.bState == 254) sym = TFT_DARKGREY;
	else if ((BleDevStE.DEV_TYP > 15) && (BleDevStE.DEV_TYP < 24) && (BleDevStE.bState == 1)) sym = TFT_WHITE;
	else if ((BleDevStE.DEV_TYP > 15) && (BleDevStE.DEV_TYP < 17) && (BleDevStE.bState == 5)) sym = TFT_YELLOW;
	else if ((BleDevStE.DEV_TYP > 16) && (BleDevStE.DEV_TYP < 24) && (BleDevStE.bState == 2)) sym = TFT_YELLOW;
	else sym = TFT_RED;
	setTextColor(sym, bkg);
        sumx += drawString("5",sumx,26,4);

	if (tmr < 41) {
	if (f_update || BleDevStA.REQ_NAME[0] || BleDevStB.REQ_NAME[0] || BleDevStC.REQ_NAME[0] || BleDevStD.REQ_NAME[0] || BleDevStE.REQ_NAME[0]) mpos = 198;
	if ((MQTT_VALP1[0]) || (MQTT_VALP4[0]) || (MQTT_VALP6[0])) {
	setTextColor(TFT_GREEN, TFT_BLACK);
  	sumx = 0;
	if (MQTT_VALP1[0]) {
	sumx += drawString(MQTT_VALP1, 0, mpos, 4);
	if (MQTT_VALP2[0] || MQTT_VALP3[0]) sumx += drawString("'/", sumx, mpos, 4);
	else sumx += drawString("' ", sumx, mpos, 4);
	}
	sumy = sumx;
	sumz = sumx;
	if (MQTT_VALP2[0] || MQTT_VALP3[0]) sumx = sumx + 54;
	if (MQTT_VALP2[0]) {
        sumy += drawString(MQTT_VALP2,sumy,mpos,2);
  	sumy += drawString("V", sumy, mpos, 2);
	}
	if (sumx > sumy) fillRect(sumy, mpos,sumx-sumy,13,TFT_BLACK);
	if (MQTT_VALP3[0]) {
        sumz += drawString(MQTT_VALP3,sumz,mpos+13,2);
  	sumz += drawString("A", sumz, mpos+13, 2);
	}
	if (sumx > sumz) fillRect(sumz, mpos+13,sumx-sumz,13,TFT_BLACK);

	if (MQTT_VALP4[0]) {
	if (!MQTT_VALP5[0]) setTextColor(TFT_GREEN, TFT_BLACK);
        else if ((!incascmp("0",MQTT_VALP5,1)) || (!incascmp("off",MQTT_VALP5,3))||
        (!incascmp("false",MQTT_VALP5,5))) {
	setTextColor(TFT_BLUE, TFT_BLACK);
	} else setTextColor(TFT_RED, TFT_BLACK);
        sumx += drawString(MQTT_VALP4,sumx, mpos,4);
  	sumx += drawString("'  ", sumx, mpos, 4);
	}
	if (MQTT_VALP6[0]) {
	setTextColor(TFT_YELLOW, TFT_BLACK);
        sumx += drawString(MQTT_VALP6,sumx,mpos,4);
	if (MQTT_VALP7[0]) {
  	sumx += drawString("'/", sumx, mpos, 4);
	sumy = sumx;
        sumx += drawString(MQTT_VALP7,sumx, mpos,2);
  	sumx += drawString("%", sumx, mpos, 2);
	fillRect(sumy, mpos+16,sumx-sumy,10,TFT_BLACK);
	} else sumx += drawString("' ", sumx, mpos, 4);
	}
	fillRect(sumx, mpos,320-sumx,26,TFT_BLACK);
	}
	if (mqttConnected) setTextColor(TFT_GREEN, TFT_BLACK);
	else if (!wf_retry_cnt) {
	if (!MQTT_SERVER[0]) setTextColor(TFT_DARKGREY, TFT_BLACK);
	else setTextColor(TFT_YELLOW, TFT_BLACK);
	} else setTextColor(TFT_RED, TFT_BLACK);
  	sumx = 0;

	if (ptr->REQ_NAME[0] && (blstnum < 5)) {
	if (!iRssiESP || (iRssiESP < -80)) sumx += drawString("\xe5 ", sumx, 224, 2);
	else if (iRssiESP < -70) sumx += drawString("\xe6 ", sumx, 224, 2);
	else if (iRssiESP < -60) sumx += drawString("\xe7 ", sumx, 224, 2);
	else sumx += drawString("\xe8 ", sumx, 224, 2);
	if (f_i2cdev & 0x60000000) {
	if (!(pwr_batmode & 0x02)) setTextColor(TFT_YELLOW, TFT_BLACK);
	else if (!(pwr_batmode & 0x01)) setTextColor(TFT_WHITE, TFT_BLACK);
	else setTextColor(TFT_GREEN, TFT_BLACK);
	if (pwr_batlevp > 94) sumx += drawString("\xe4 ", sumx, 224, 2);
	else if (pwr_batlevp > 74) sumx += drawString("\xe3 ", sumx, 224, 2);
	else if (pwr_batlevp > 49) sumx += drawString("\xe2 ", sumx, 224, 2);
	else if (pwr_batlevp > 24) sumx += drawString("\xe1 ", sumx, 224, 2);
	else sumx += drawString("\xe0 ", sumx, 224, 2);
	}
	if (ptr->btauthoriz) {
	if (!ptr->bState && ptr->bHeat) setTextColor(TFT_YELLOW, TFT_BLACK);
	else if (!ptr->bState) setTextColor(TFT_BLUE, TFT_BLACK);
	else if (ptr->bState == 254) setTextColor(TFT_DARKGREY, TFT_BLACK);
	else if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 24) && (ptr->bState == 1)) setTextColor(TFT_WHITE, TFT_BLACK);
	else if ((ptr->DEV_TYP > 15) && (ptr->DEV_TYP < 17) && (ptr->bState == 5)) setTextColor(TFT_YELLOW, TFT_BLACK);
	else if ((ptr->DEV_TYP > 16) && (ptr->DEV_TYP < 24) && (ptr->bState == 2)) setTextColor(TFT_YELLOW, TFT_BLACK);
	else if ((ptr->DEV_TYP == 74) && !(ptr->bState & 0x01)) setTextColor(TFT_BLUE, TFT_BLACK);
	else setTextColor(TFT_RED, TFT_BLACK);
	sumx += drawString(ptr->DEV_NAME, sumx, 224, 2);
	sumx += drawString(":", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	if (!ptr->DEV_TYP) {
	sumx += drawString(" Not Defined, ", sumx, 224, 2);
	} else if ((ptr->DEV_TYP < 10) || ((ptr->DEV_TYP > 63) && (ptr->DEV_TYP < 73))) {
	sumx += drawString(" T: ", sumx, 224, 2);
	if (ptr->bCtemp > 84) setTextColor(TFT_WHITE, TFT_BLACK);
	else if (ptr->bCtemp > 64) setTextColor(TFT_YELLOW, TFT_BLACK);
	else if (ptr->bCtemp == 0) setTextColor(TFT_DARKGREY, TFT_BLACK);
	itoa(ptr->bCtemp,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("'", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	sumx += drawString("/", sumx, 224, 2);
	if (ptr->bHtemp > 84) setTextColor(TFT_WHITE, TFT_BLACK);
	else if (ptr->bHtemp > 39) setTextColor(TFT_YELLOW, TFT_BLACK);
	itoa(ptr->bHtemp,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("'", sumx, 224, 2);
	if (ptr->DEV_TYP > 3) {
        setTextColor(TFT_GREEN, TFT_BLACK);
	sumx += drawString(", V: ", sumx, 224, 2);
	if (ptr->bCVol < 250) {
	if (!volperc) {
	itoa(ptr->bCVol / 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString(".", sumx, 224, 2);
	itoa(ptr->bCVol % 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	} else {
	itoa(ptr->bCVol,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	}	
	} else if (ptr->bCVol == 253) sumx += drawString("???", sumx, 224, 2);
	else if (ptr->bC1temp && ptr->bS1Energy) sumx += drawString("??", sumx, 224, 2);
	else sumx += drawString("?", sumx, 224, 2);
	sumx += drawString("/", sumx, 224, 2);
	if (!volperc) {
	itoa(ptr->bCVoll / 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString(".", sumx, 224, 2);
	itoa(ptr->bCVoll % 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("l", sumx, 224, 2);
	} else {
	itoa(ptr->bCVoll,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("%", sumx, 224, 2);
	}	
	}
	} else if (ptr->DEV_TYP < 12) {
	sumx += drawString(" Lock: ", sumx, 224, 2);
	if (ptr->bLock) {
        setTextColor(TFT_RED, TFT_BLACK);
	sumx += drawString("On", sumx, 224, 2);
	} else sumx += drawString("Off", sumx, 224, 2);
        if (ptr->DEV_TYP == 11) {
	sumx += drawString(", Pwr: ", sumx, 224, 2);
	itoa(ptr->bHtemp,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	}
	} else if (ptr->DEV_TYP < 16) {
	sumx += drawString(" Lock: ", sumx, 224, 2);
	if (ptr->bLock) {
        setTextColor(TFT_RED, TFT_BLACK);
	sumx += drawString("On", sumx, 224, 2);
	} else sumx += drawString("Off", sumx, 224, 2);
	sumx += drawString(", Strength: ", sumx, 224, 2);
	if (ptr->bAwarm) {
        setTextColor(TFT_RED, TFT_BLACK);
	sumx += drawString("On", sumx, 224, 2);
	} else sumx += drawString("Off", sumx, 224, 2);
	} else if (ptr->DEV_TYP < 60) {
	sumx += drawString(" P: ", sumx, 224, 2);
	itoa(ptr->bProg,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("/", sumx, 224, 2);
	itoa(ptr->bModProg,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	if (ptr->DEV_TYP != 48) {
	sumx += drawString("/", sumx, 224, 2);
	itoa(ptr->bHtemp,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	if (ptr->DEV_TYP == 52) sumx += drawString("%", sumx, 224, 2);
	else sumx += drawString("'", sumx, 224, 2);
	}
	sumx += drawString(", T: ", sumx, 224, 2);
	itoa(ptr->bCHour,buff,10);
	if (ptr->bCHour < 10) sumx += drawString("0", sumx, 224, 2);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString(":", sumx, 224, 2);
	itoa(ptr->bCMin,buff,10);
	if (ptr->bCMin < 10) sumx += drawString("0", sumx, 224, 2);
	sumx += drawString(buff, sumx, 224, 2);
	} else if (ptr->DEV_TYP == 60) {
	sumx += drawString(" Lock: ", sumx, 224, 2);
	if (ptr->bLock) {
        setTextColor(TFT_RED, TFT_BLACK);
	sumx += drawString("On", sumx, 224, 2);
	} else sumx += drawString("Off", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	sumx += drawString(", Pos: ", sumx, 224, 2);
	switch (ptr->bModProg) {
	case 0:
	sumx += drawString("Work", sumx, 224, 2);
	break;
	case 1:
	sumx += drawString("Stnd", sumx, 224, 2);
	break;
	case 2:
	sumx += drawString("Offs", sumx, 224, 2);
	break;
	case 3:
	sumx += drawString("Upsd", sumx, 224, 2);
	break;
	case 254:
	sumx += drawString("Offl", sumx, 224, 2);
	break;
	}
	} else if ((ptr->DEV_TYP > 60) && (ptr->DEV_TYP < 63)) {
	sumx += drawString(" T: ", sumx, 224, 2);
	if (ptr->bSEnergy & 0x80000000) sumx += drawString("-", sumx, 224, 2);
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("', B: ", sumx, 224, 2);
	itoa(ptr->bCtemp,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("%", sumx, 224, 2);
	if (ptr->bProg) {
	sumx += drawString(", ", sumx, 224, 2);
	setTextColor(TFT_RED, TFT_BLACK);
	if (ptr->DEV_TYP == 61) sumx += drawString("Open", sumx, 224, 2);
	else sumx += drawString("Smoke", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	}
	} else if (ptr->DEV_TYP == 63) {
	sumx += drawString(" T/H/P: ", sumx, 224, 2);
	if (ptr->bSEnergy & 0x80000000) sumx += drawString("-", sumx, 224, 2);
	itoa((ptr->bSEnergy & 0x7fffffff) / 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("'/", sumx, 224, 2);
	itoa(ptr->bSHum / 10,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("%/", sumx, 224, 2);
	itoa(ptr->bSTime * 75 / 10000,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("mmHg", sumx, 224, 2);
	} else if (ptr->DEV_TYP == 73) {
	sumx += drawString(" T: ", sumx, 224, 2);
	itoa(ptr->bCHour,buff,10);
	if (ptr->bCHour < 10) sumx += drawString("0", sumx, 224, 2);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString(":", sumx, 224, 2);
	itoa(ptr->bCMin,buff,10);
	if (ptr->bCMin < 10) sumx += drawString("0", sumx, 224, 2);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString(":", sumx, 224, 2);
	itoa(ptr->bDMin,buff,10);
	if (ptr->bDMin < 10) sumx += drawString("0", sumx, 224, 2);
	sumx += drawString(buff, sumx, 224, 2);
	} else if ((ptr->DEV_TYP > 73) && (ptr->DEV_TYP < 76)) {
	sumx += drawString(" P/I/B: ", sumx, 224, 2);
	if (ptr->bProg > 90) setTextColor(TFT_WHITE, TFT_BLACK);
	else if (ptr->bProg) setTextColor(TFT_YELLOW, TFT_BLACK);
	itoa(ptr->bProg,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("%", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	sumx += drawString("/", sumx, 224, 2);
	if (ptr->bSEnergy > 80) setTextColor(TFT_WHITE, TFT_BLACK);
	else if (ptr->bSEnergy) setTextColor(TFT_YELLOW, TFT_BLACK);
	itoa(ptr->bSEnergy,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("%", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	sumx += drawString("/", sumx, 224, 2);
	if (ptr->bCtemp < 15) setTextColor(TFT_RED, TFT_BLACK);
	else if (ptr->bCtemp < 40) setTextColor(TFT_YELLOW, TFT_BLACK);
	itoa(ptr->bCtemp,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("%", sumx, 224, 2);
        setTextColor(TFT_GREEN, TFT_BLACK);
	} else if (ptr->DEV_TYP == 76) {
	sumx += drawString(" ", sumx, 224, 2);
	setTextColor(TFT_RED, TFT_BLACK);
        if (ptr->bLock & 0x01) {
	setTextColor(TFT_RED, TFT_BLACK);
	sumx += drawString("Motion", sumx, 224, 2);
        } else if (ptr->bLock & 0x02) {
	setTextColor(TFT_YELLOW, TFT_BLACK);
	sumx += drawString("Presence", sumx, 224, 2);
	}
	}
        setTextColor(TFT_GREEN, TFT_BLACK);
	sumx += drawString(", ", sumx, 224, 2);
	if (ptr->iRssi < -100) setTextColor(TFT_WHITE, TFT_BLACK);
	else if (ptr->iRssi < -90) setTextColor(TFT_YELLOW, TFT_BLACK);
	itoa(ptr->iRssi,buff,10);
	sumx += drawString(buff, sumx, 224, 2);
	sumx += drawString("dB", sumx, 224, 2);
	fillRect(sumx,224,320-sumx,16,TFT_BLACK);
	} else {
	setTextColor(TFT_DARKGREY, TFT_BLACK);
	if (ptr->DEV_NAME[0]) strcpy(buf,ptr->DEV_NAME);
	else {
	strcpy(buf,"Ble");
	itoa(blstnum1,buff,10);
	strcat(buf,buff);
	}
	strcat(buf,": Offline");
	if (FND_NAME[0]) {
	strcat(buf,", found: ");
	strcat(buf,FND_NAME);
	}
	sumx += drawString(buf, sumx, 224, 2);
	fillRect(sumx,224,320-sumx,16,TFT_BLACK);
	}
	}
	if (blstnum == 254) {
	setTextColor(TFT_WHITE, TFT_BLACK);
	sumx = drawString("Update firmware in progress ...", 0, 224, 2);
	fillRect(sumx,224,320-sumx,16,TFT_BLACK);
	setTextColor(TFT_GREEN, TFT_BLACK);
	}
	} //tmr
}

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
	MyJPGbufidx = -1;
            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
	MyJPGbufidx = 0;
            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
	MyJPGbufidx = 0;
//            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
//            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_ON_HEADER");
	MyJPGbufidx = 0;
//            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
//            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
	if ((MyJPGbufidx >= 0) && (MyJPGbufidx+evt->data_len < MyJPGbuflen)) {
	memcpy(MyJPGbuf + MyJPGbufidx, (char*)evt->data, evt->data_len);
	MyJPGbufidx +=evt->data_len;
	} else MyJPGbufidx = -1;

//                printf("%.*s", evt->data_len, (char*)evt->data);
//        if (fdebug) esp_log_buffer_hex(AP_TAG, (char*)evt->data, evt->data_len);

	}

            break;
        case HTTP_EVENT_ON_FINISH:
//            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
//            if (fdebug) ESP_LOGI(AP_TAG, "HTTP_EVENT_DISCONNECTED");
            break;
	}
	if (MyJPGbufidx >=0) return ESP_OK;
	else return ESP_ERR_NO_MEM;	
}

//Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct {
	char *inData; //Pointer to jpeg data
	uint16_t inPos;              //Current position in jpeg data
	uint16_t *outData;          //Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
	int outW;                    //Width of the resulting file
	int outH;                    //Height of the resulting file
} JpegDev;

//Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure.
static uint16_t infunc(JDEC *decoder, uint8_t *buf, uint16_t len)
{
    //Read bytes from input file
    JpegDev *jd = (JpegDev *)decoder->device;
	uint16_t llen = len;
	if ((buf != NULL) && (len > 0)) {
	for (int i = 0; i < len; i++) buf[i] = 0;
	if (jd->inPos + len < MyJPGbuflen) {
        memcpy(buf, jd->inData + jd->inPos, len);
	} else llen = 0;
	}
	jd->inPos += len;
	return llen;
}

//Output jpeg function. Re-encodes the RGB888 data from the jpeg decoder as big-endian RGB565 and
//stores it in the outData array of the JpegDev structure.
static uint16_t outfunc(JDEC *decoder, void *bitmap, JRECT *rect)
{
	int result = 1;
	esp_err_t ret;
	char swdata[4];
	spi_transaction_t t;
	uint16_t buf[520];
	uint16_t ycorr;
//	JpegDev *jd = (JpegDev *)decoder->device;
	uint8_t *in = (uint8_t *)bitmap;
	int i = 0;
	for (int y = rect->top; y <= rect->bottom; y++) {
	for (int x = rect->left; x <= rect->right; x++) {
            //We need to convert the 3 bytes in `in` to a rgb565 value.
            uint16_t v = 0;
            v |= ((in[0] >> 3) << 11);
            v |= ((in[1] >> 2) << 5);
            v |= ((in[2] >> 3) << 0);
            //The LCD wants the 16-bit value in big-endian, so swap bytes
            v = (v >> 8) | (v << 8);
	buf[i] = v;
	i++;		
	if (i > 256) {
	i = 256;
	result = 0;
	}
	in += 3;
        }
    }

	if (result) {
	int j = i;
  	uint8_t *bufp = (uint8_t*)buf;
	swdata[0] = 0x2a;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	swdata[0] = (rect->left + xoffs)>>8 & 0xff;   //Start Col High;
	swdata[1] = (rect->left + xoffs) & 0xff;      //Start Col Low;
	swdata[2] = (rect->right + xoffs)>>8 & 0xff;   //End Col High;
	swdata[3] = (rect->right + xoffs)& 0xff;      //End Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*4;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
//
	swdata[0] = 0x2b;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	ycorr = rect->top + 52;
	swdata[0] = (ycorr)>>8 & 0xff;   //Start Col High;
	swdata[1] = (ycorr) & 0xff;      //Start Col Low;
	ycorr = rect->bottom + 52;
	swdata[2] = (ycorr)>>8 & 0xff;   //End Col High;
	swdata[3] = (ycorr)& 0xff;      //End Col Low;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*4;                   //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)1;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
//
	swdata[0] = 0x2c;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8;                     //Command is 8 bits
	t.tx_buffer=&swdata;            //The data is the cmd itself
	t.user=(void*)0;                //D/C needs to be set to 0
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
//
	while (j > 0) {
	i = j;
	if (i > 32) i = 32;
	memset(&t, 0, sizeof(t));       //Zero out the transaction
	t.length=8*2*i;
	t.tx_buffer=bufp;
	t.user=(void*)1;                //D/C to 1
	ret=spi_device_polling_transmit(spi, &t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	j -= i;
	bufp += i*2;
	}
	}


//  end_tft_write();


//if (fdebug) ESP_LOGI(AP_TAG, "Count: %d",count);

    return result;
}




//Size of the work space for the jpeg decoder.
#define WORKSZ 6144

bool tftjpg()
{
	bool result = false;
	MyJPGbuf = NULL;
	char *JpHttpUri = NULL;
	int  jstat = 0;
	int  jlen = 0;
	xoffs = 0;
	MyJPGbufidx = 0;
	if (MyHttpUri[0] && jpg_time && (parsoff(MyHttpUri, "http://", 9) || parsoff(MyHttpUri, "https://", 10))) {
	MyJPGbuf = malloc(MyJPGbuflen);
	if (MyJPGbuf == NULL) {
        if (fdebug) ESP_LOGE(AP_TAG, "Tftjpg: No memory");
	MemErr++;
	if (!MemErr) MemErr--;
	} else {	
	memset(MyJPGbuf,0,MyJPGbuflen);
	esp_err_t err = ESP_OK;
	JpHttpUri = calloc(520, 1);
	if (JpHttpUri != NULL) {
	memset(JpHttpUri,0,520);
	myurlcpy(JpHttpUri, MyHttpUri, 518);
	if (fdebug) ESP_LOGI(AP_TAG, "Url: %s",JpHttpUri);
	esp_http_client_config_t config = {
	.url = JpHttpUri,
	.event_handler = _http_event_handle,

	.max_authorization_retries = 3,
//	.crt_bundle_attach = esp_crt_bundle_attach,
//	.use_global_ca_store = true,

	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_http_client_set_url(client, JpHttpUri);
	esp_http_client_set_header(client, "Content-Type", "image/jpeg");
	err = esp_http_client_perform(client);
	if (err == ESP_OK) {
	jstat = esp_http_client_get_status_code(client);
	jlen = esp_http_client_get_content_length(client);
	if (fdebug) ESP_LOGI(AP_TAG, "Status = %d, content_length = %d, offset_length = %d",
           jstat,  jlen, MyJPGbufidx);
	}
	esp_http_client_cleanup(client);
	free(JpHttpUri);
	JpgLoad++;
	} else MyJPGbufidx = -1;
	if ((err != ESP_OK) || (jlen == -1)) {
	pushImage(0, 52, 320, 240, wallpaper);
	setTextColor(TFT_YELLOW, TFT_BLACK);
       	drawString("HTTP(S) connection error", 8, 60, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
	JpgLoadErr++;
	result = true;
	} else if (MyJPGbufidx == -1) {
  	uint32_t sumx;
	char buf[16];
	pushImage(0, 52, 320, 240, wallpaper);
	setTextColor(TFT_YELLOW, TFT_BLACK);
	itoa(jlen,buf,10);
       	drawString("Not enough memory:", 8, 60, 4);
	sumx = 40;
       	sumx += drawString(buf, sumx, 90, 4);
       	sumx += drawString(" -> ", sumx, 90, 4);
	itoa(MyJPGbuflen,buf,10);
       	sumx += drawString(buf, sumx, 90, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
	JpgLoadErr++;
	result = true;
	} else if ((err == ESP_OK) && jstat && (jstat != 200)) {
	pushImage(0, 52, 320, 240, wallpaper);
	setTextColor(TFT_YELLOW, TFT_BLACK);
	strcpy(MyJPGbuf,"Status:  ");
	itoa(jstat,MyJPGbuf+8,10);
       	drawString(MyJPGbuf, 8, 60, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
	JpgLoadErr++;
	result = true;
	} else if ((err == ESP_OK) && jstat && (MyJPGbufidx > 15) && (MyJPGbufidx < MyJPGbuflen)) {	
	if ((!memcmp(&MyJPGbuf[0],"\xff\xd8\xff",3)) && (MyJPGbufidx > 128)) {
	char *work = NULL;
	JDEC decoder;
	JpegDev jd;
	esp_err_t ret = ESP_OK;

    //Allocate the work space for the jpeg decoder.
	work = calloc(WORKSZ, 1);
	if (work == NULL) {
        if (fdebug) ESP_LOGE(AP_TAG, "Cannot allocate workspace");
        ret = ESP_ERR_NO_MEM;
	pushImage(0, 52, 320, 240, wallpaper);
	setTextColor(TFT_YELLOW, TFT_BLACK);
       	drawString("JPEG: no memory", 8, 60, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
	MemErr++;
	if (!MemErr) MemErr--;
	result = true;
	} else {	
    //Populate fields of the JpegDev struct.
	jd.inData = MyJPGbuf;
	jd.inPos = 0;
	jd.outData = NULL;
	jd.outW = 320; //IMAGE_W;
	jd.outH = 176; //IMAGE_H;
    //Prepare and decode the jpeg.
	ret = jd_prepare(&decoder, infunc, work, WORKSZ, (void *)&jd);
	if (ret != JDR_OK) {
        if (fdebug) ESP_LOGE(AP_TAG, "Image decoder: jd_prepare failed (%d)", ret);
	pushImage(0, 52, 320, 240, wallpaper);
	setTextColor(TFT_YELLOW, TFT_BLACK);
       	drawString("JPEG: not supported", 8, 60, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
	result = true;
	} else {
	uint8_t scale;
	if (decoder.width > 640) {
	scale = 2;
	xoffs = (320 - (decoder.width >> 2)) >> 1;
	} else if (decoder.width > 320) {
	scale = 1;
	xoffs = (320 - (decoder.width >> 1)) >> 1;
	} else {
	scale = 0;
	xoffs = (320 - decoder.width) >> 1;
	}
	if (MyHttpMqtt & 0x40) {
	fillRect(0,52,320,240,TFT_BLACK);
        MyHttpMqtt = MyHttpMqtt & 0xbf;
	}
	ret = jd_decomp(&decoder, outfunc, scale);
	if (ret != JDR_OK) {
        if (fdebug) ESP_LOGE(AP_TAG, "Image decoder: jd_decode failed (%d)", ret);
	setTextColor(TFT_YELLOW, TFT_BLACK);
       	drawString("JPEG: decoder error", 8, 60, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
	result = true;
	} else {
//	if (fdebug) ESP_LOGI(AP_TAG, "Image decoder: jd_decode Ok");
	result = true;
	}
	}
	free(work);
	}
	} else {
	uint16_t htoff = 0;
//	if (MyHttpMqtt & 0x40) {
	fillRect(0,52,320,240,TFT_BLACK);
        MyHttpMqtt = MyHttpMqtt & 0xbf;
//	}
	setTextColor(TFT_WHITE, TFT_BLACK);
	htoff = parsoff(MyJPGbuf ,"<body", MyJPGbuflen);
	if(htoff) {	
	if (htoff > 5) htoff = htoff - 5;
       	drawHString(MyJPGbuf + htoff, 0, 60, 2);
	} else drawString(MyJPGbuf, 0, 60, 2); 

/*
	pushImage(0, 52, 320, 240, wallpaper);
	setTextColor(TFT_YELLOW, TFT_BLACK);
       	drawString("Content not supported", 8, 60, 4);
        MyHttpMqtt = MyHttpMqtt | 0x40;
*/
//if (fdebug) ESP_LOGI(AP_TAG, "String: %s",MyJPGbuf);
//if (fdebug) esp_log_buffer_hex(AP_TAG, MyJPGbuf, 256);
	result = true;
}        //jpg
}        //load ok
	free(MyJPGbuf);
}        //alloc

} else if (MyHttpUri[0] && jpg_time) { //if not uri
	if (MyHttpMqtt & 0x40) {
//	fillRect(0,52,320,240,TFT_WHITE);
	fillRect(0,52,320,240,TFT_BLACK);
        MyHttpMqtt = MyHttpMqtt & 0xbf;
	}
	setTextColor(TFT_WHITE, TFT_BLACK);
       	drawString(MyHttpUri , 0, 60, 2);
	result = true;
}        //uri&time
	return result;
}



