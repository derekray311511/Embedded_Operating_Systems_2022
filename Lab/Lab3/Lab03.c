#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>

#define MAX_LEN 40

// show the result from 0-255 integer
void show_result(int result, int *fd){
    printf("Showing %d on LED and 7 SEG\n", result);
    _7seg_info_t seg_data;
    int i = 0, temp = result;
    unsigned long hex_result = 0;
    for (i = 1; i <= 4096; i*=16){
        // printf("%d^%d, ", temp % 10, i);
        hex_result += (temp % 10) * i;
        temp /= 10;
    }
    printf("HEX: %04lx\n", hex_result);

    ioctl(*fd, _7SEG_IOCTL_ON, NULL);
    seg_data.Mode = _7SEG_MODE_HEX_VALUE;
    seg_data.Which = _7SEG_ALL;
    seg_data.Value = hex_result;
    ioctl(*fd, _7SEG_IOCTL_SET, &seg_data);

    temp = result;
    int pos, vis = 0;
    for (i = 0; i < 8; i++){
        pos = 7 - i;
        vis += (temp % 2) * pow(10, i);
        if (temp % 2)
            ioctl(*fd, LED_IOCTL_BIT_SET, &pos);
        else
            ioctl(*fd, LED_IOCTL_BIT_CLEAR, &pos);
        temp /= 2;
    }
    printf("LED: %d\n", vis);

    return;
}

char interpret(unsigned short k){
    switch(k & 0xff){
        case 'A': return '+';
        case 'B': return '-';
        case 'C': return '*';
        case 'D': return '/';
        default: return (k & 0xff);
    }
}

int calculator(char *s) {
  int ans = 0, curAns = 0;
  int num = 0, n = strlen(s);
  char op = '+';
  int i = 0;
  for (i = 0; i < n; ++i) {
    char c = s[i];
    if (isdigit(c)) {
      num = num * 10 + (c - '0');
    }
    if (c == '+' || c == '-' || c == '*' || c == '/' || i == n - 1) {
      switch (op) {
        case '+': 
          curAns += num; 
          break;
        case '-': 
          curAns -= num; 
          break;
        case '*': 
          curAns *= num; 
          break;
        case '/': 
          curAns /= num; 
          break;
      }
      if (c == '+' || c == '-' || i == n - 1) {
        ans += curAns;
        curAns = 0;
      }
      op = c;
      num = 0;
    } 
  }
  return ans;
}

int main(int argc, char *argv[]){
    int fd, ret;  // file descriptor for /dev/lcd
    int result;   // The result of calculation
    char s[MAX_LEN];
    int s_count = 0;
    unsigned short data, key;

    // Open device /dev/lcd
    if ((fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("Open /dev/lcd failed.\n");
        exit(-1);
    }
    ioctl(fd, KEY_IOCTL_CLEAR, key);
    while(1){
        ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
        // ret < 0 means no input
        if (ret < 0){
            sleep(1);
            continue;
        }
        ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
        unsigned short key_value = (key & 0xff);
        // printf("key: %d\n", key);

        if (key_value == '#'){
            s[s_count++] = '\0';
            result = calculator(s);
            s[0] = '\0';
            s_count = 0;
            show_result(result, &fd);
        }
        else if (key_value == '*') {
            s[0] = '\0';
            s_count = 0;
        }
        else if (key_value <= '9' && key_value >= '0') {
            s[s_count++] = key_value;
            s[s_count] = '\0';
        }
        else if (key_value <= 'D' && key_value >= 'A') {
            s[s_count++] = interpret(key_value);
            s[s_count] = '\0';
        }
        else{
            printf("key out of range.\n");
        }
        
        lcd_write_info_t display;  //struct for saving LCD data
        // Clear LCD
		ioctl(fd, LCD_IOCTL_CLEAR, NULL);
		// Save output string to display data structure
		display.Count = sprintf((char *)display.Msg, s);
		// print msg to LCD
		ioctl(fd, LCD_IOCTL_WRITE, &display);
    }
    ioctl(fd, _7SEG_IOCTL_OFF, NULL);
    // Close fd
    close(fd);
    return 0;
}