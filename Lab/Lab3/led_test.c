// Commands:

// LED_IOCTL_SET // set the specified LED (D9 - D16)
// LED_IOCTL_CLEAR // clear the specified LED (D9 - D16)

// _7SEG_IOCTL_ON: turn on 7 segment LED (no data is needed)
// _7SEG_IOCTL_OFF: turn off 7 segment LED (no data is needed)
// _7SEG_IOCTL_SET: set 7 segment LED (_7seg_info_t)

// KEY_IOCTL_GET_CHAR: unsigned short, get its ASCII value/
// KEY_IOCTL_WATI_CHAR: wait until get a character.
// KEY_IOCTL_CHECK_EMPTY
// KEY_IOCTL_CLEAR
// KEY_IOCTL_CANCEL_WAIT_CHAR

/*
// Clear LCD data and move cursor back to the Upper-left corner
#define LCD_IOCTL_CLEAR		LCD_IO (0x0)

// Write char to LCD
#define LCD_IOCTL_WRITE		LCD_OPW (0x01, lcd_write_info_t)

// Turn on or off cursor
#define LCD_IOCTL_CUR_ON	LCD_IO (0x02)
#define LCD_IOCTL_CUR_OFF   LCD_IO (0x03)

// Get and Set the position (X, Y) of cursor
#define LCD_IOCTL_CUR_GET 	LCD_IOR (0x04, lcd_write_info_t)
#define LCD_IOCTL_CUR_SET	LCD_IOW (0x05, lcd_write_info_t)

// Write a picture to LCD
#define LCD_IOCTL_DRAW_FULL_IMAGE LCD_IOW (0x06, lcd_full_image_info_t)
*/


// Values:

// LED_ALL_ON          0xFF
// LED_ALL_OFF	       0x00
// LED_D9_INDEX        1
// LED_D10_INDEX       2
// LED_D11_INDEX       3
// LED_D12_INDEX       4
// LED_D13_INDEX       5
// LED_D14_INDEX       6
// LED_D15_INDEX       7
// LED_D16_INDEX       8

/*
#define VK_S2 1		ASCII = '1'
#define VK_S3 2		ASCII = '2'
#define VK_S4 3 	ASCII = '3'
#define VK_S5 10 	ASCII = 'A'
#define VK_S6 4
#define VK_S7 5
#define VK_S8 6
#define VK_S9 11
#define VK_S10 7
#define VK_S11 8
#define VK_S12 9
#define VK_S13 12
#define VK_S14 14	ASCII = '*'
#define VK_S15 0
#define VK_S16 15 	ASCII = '#'
#define VK_S17 13
*/

#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

/*
typedef struct _7Seg_Info {
	unsigned char Mode ; // _7SEG_MODE_PATTERN or _7SEG_MODE_HEX_VALUE
	unsigned char Which ; // D5 ~ D8
	unsigned long Value ; // pattern or hex
} _7seg_info_t;

// Data structure for writing char to LCD screen
typedef struct lcd_write_info{
	unsigned char Msg[512];		// the array for saving input
	unsigned short Count;		// the number of input char
	int CursorX, CursorY;		// X, Y axis of cursor
} lcd_write_info_t;

// Data structure for writing a picture to LCD screen
typedef struct lcd_full_image_info{
	unsigned short data[0x800];	// the array for saving picture
} lcd_full_image_info_t;
*/

int main(int argc, char *argv[]){
	int Test_LED = 0;
	int Test_7seg = 0;
	int Test_keypad = 1;
	int Test_LCD = 1;

	int fd;  // file descriptor for /dev/lcd

	if (Test_LED){
		// Testing LED
		int retval;
		unsigned short data;
		// Open device /dev/lcd
		if ((fd = open("/dev/lcd", O_RDWR)) < 0){
			printf("Open /dev/lcd failed.\n");
			exit(-1);
		}
		// Turn on all LED lamps
		data = LED_ALL_ON;
		ioctl(fd, LED_IOCTL_SET, &data);
		printf("Turn on all LED lamps\n");
		sleep(1);
		// Turn off all LED lamps
		data = LED_ALL_OFF;
		ioctl(fd, LED_IOCTL_SET, &data);
		printf("Turn off all LED lamps\n");
		sleep(1);
		// Turn on D9
		data = LED_D9_INDEX;
		ioctl(fd, LED_IOCTL_BIT_SET, &data);
		printf("Turn on D9\n");
		sleep(1);
		// Turn off D9 
		data = LED_D9_INDEX;
		ioctl(fd, LED_IOCTL_BIT_CLEAR, &data);
		printf("Turn off D9\n");
		sleep(1);
		// Close fd
		close(fd);
		printf("close LED\n");
	}

	if (Test_7seg){
		// Testing 7 seg
		_7seg_info_t seg_data;
		if ((fd = open("/dev/lcd", O_RDWR)) < 0){
			return -1;
		}
		ioctl(fd, _7SEG_IOCTL_ON, NULL);
		seg_data.Mode = _7SEG_MODE_HEX_VALUE;
		seg_data.Which = _7SEG_ALL;
		seg_data.Value = 0x2004;
		ioctl(fd, _7SEG_IOCTL_SET, &seg_data);
		printf("HEX mode, showing 2004\n");
		sleep(1);
		seg_data.Mode = _7SEG_MODE_PATTERN;
		seg_data.Which = _7SEG_D5_INDEX | _7SEG_D8_INDEX;
		seg_data.Value = 0x6d7f;	// change to 5008
		ioctl(fd, _7SEG_IOCTL_SET, &seg_data);
		printf("PATTERN mode, showing 5008\n");
		sleep(1);
		ioctl(fd, _7SEG_IOCTL_OFF, NULL);
		close(fd);
		printf("close 7 SEG\n");
	}

	if (Test_keypad){
		// Testing keypad
		unsigned short key;
		int ret;

		if ((fd = open("/dev/lcd", O_RDWR)) < 0)
			return -1;
		
		ioctl(fd, KEY_IOCTL_CLEAR, key);
		while(1){
			ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
			// ret < 0 means no input
			if (ret < 0){
				sleep(1);
				continue;
			}
			ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
			printf("key: %d\n", key);
			if ((key & 0xff) == '#')
				break;
		}
		close(fd);
		printf("close keypad\n");
	}

	if (Test_LCD){
		// Testing LCD
		lcd_write_info_t display;  //struct for saving LCD data
		if ((fd = open("/dev/lcd", O_RDWR)) < 0){
			printf("open /dev/lcd error\n");
			return -1;
		}
		// Clear LCD
		ioctl(fd, LCD_IOCTL_CLEAR, NULL);
		// Save output string to display data structure
		display.Count = sprintf((char *)display.Msg, "Hello world\n");
		// print out "Hello world" to LCD
		ioctl(fd, LCD_IOCTL_WRITE, &display);

		// Get the cursor position
		ioctl(fd, LCD_IOCTL_CUR_GET, &display);
		printf("The cursor position is at (x, y) = (%d, %d)\n",
				display.CursorX, display.CursorY);

		char string[20];
		scanf("%s", string);
		display.Count = sprintf((char *)display.Msg, string);
		ioctl(fd, LCD_IOCTL_WRITE, &display);
		ioctl(fd, LCD_IOCTL_CUR_GET, &display);
		printf("The cursor position is at (x, y) = (%d, %d)\n",
				display.CursorX, display.CursorY);
		close(fd);
		printf("close LCD\n");
	}

	return 0;
}
