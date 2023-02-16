#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>

#define MAX_LEN 80

// show the 7SEG (money to pay)
void show_7SEG(int result, int *fd){
    printf("Showing %d on 7 SEG\n", result);
    _7seg_info_t seg_data;
    int i = 0, temp = result;
    unsigned long hex_result = 0;
    for (i = 1; i <= 4096; i*=16){
        hex_result += (temp % 10) * i;
        temp /= 10;
    }
    // Check HEX number
    // printf("HEX: %04lx\n", hex_result);

    ioctl(*fd, _7SEG_IOCTL_ON, NULL);
    seg_data.Mode = _7SEG_MODE_HEX_VALUE;
    seg_data.Which = _7SEG_ALL;
    seg_data.Value = hex_result;
    ioctl(*fd, _7SEG_IOCTL_SET, &seg_data);

    return;
}

// Show the LED by distance
void show_LED(int num, int *fd){
    int i = 0;
    if (num > 8)
        printf("Error: 0 <= num <= 8\n");
    for (; i < num; i++){
        ioctl(*fd, LED_IOCTL_BIT_SET, &i);
    }
    for (i = num; i < 8; i++){
        ioctl(*fd, LED_IOCTL_BIT_CLEAR, &i);
    }
}

// Show LCD monitor
void show_LCD(char *string, int *fd){
    lcd_write_info_t display;  //struct for saving LCD data
    // Clear LCD
    ioctl(*fd, LCD_IOCTL_CLEAR, NULL);
    // Save output string to display data structure
    display.Count = sprintf((char *)display.Msg, string);
    // print msg to LCD
    ioctl(*fd, LCD_IOCTL_WRITE, &display);
}

char *START_MENU = "\
1. shop list\n\
2. order\n\0";

char *SHOP_LIST1 = "\
dessert shop: 3km\n\
Beverage Shop: 5km\n\
Diner: 8km\n\n\
Press any key to go back\n\0";

char *SHOP_LIST2 = "\
1. dessert shop: 3km\n\
2. Beverage Shop: 5km\n\
3. Diner: 8km\n\0";

int distance[3] = {3, 5, 8};

// Dessert_menu
char *Dessert_menu = "\
1. cookie: $60\n\
2. cake: $80\n\
3. confirm\n\
4. cancel\n\0";
// Beverage_menu
char *Beverage_menu = "\
1. tea: $40\n\
2. boba: $70\n\
3. confirm\n\
4. cancel\n\0";
// Diner_menu
char *Diner_menu = "\
1. fired rice: $120\n\
2. Egg-drop soup: $50\n\
3. confirm\n\
4. cancel\n\0";

unsigned short int cost[3][2] = {{60, 80}, {40, 70}, {120, 50}};


int main(int argc, char *argv[]){
    char *menu[3];
    menu[0] = Dessert_menu;
    menu[1] = Beverage_menu;
    menu[2] = Diner_menu;
    int fd, ret;  // file descriptor for /dev/lcd
    char s[MAX_LEN];
    unsigned short key;
    int money;

    // Open device /dev/lcd
    if ((fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("Open /dev/lcd failed.\n");
        exit(-1);
    }
    // initial display
    // Clear LCD
    ioctl(fd, KEY_IOCTL_CLEAR, key);
    ioctl(fd, LCD_IOCTL_CLEAR, NULL);
    show_LED(8, &fd);
    show_7SEG(0000, &fd);
    show_LCD(START_MENU, &fd);
    // Start running app
    while(1){
        // Read key
        ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
        // ret < 0 means no input
        if (ret < 0){
            sleep(1);
            continue;
        }
        ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
        unsigned short key_value = (key & 0xff);
        // printf("key: %d\n", key);

        // initialize money
        printf("Start menu\n");
        money = 0;
        show_7SEG(0000, &fd);

        // Jump to shop list mode
        if (key_value == '1'){
            strcpy(s, "1\0");
            show_LCD(s, &fd);
            // Wait for command check by '#'
            while(1){
                // Read key
                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                // ret < 0 means no input
                if (ret < 0){
                    sleep(1);
                    continue;
                }
                ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                unsigned short key_value = (key & 0xff);
                if (key_value == '#'){
                    break;
                }
            }
            strcpy(s, SHOP_LIST1);
            show_LCD(s, &fd);
            while(1){
                // Wait for any key to go back
                // Read key
                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                // ret < 0 means no input
                if (ret < 0){
                    sleep(1);
                    continue;
                }
                else{
                    break;
                }
            }
            strcpy(s, START_MENU);
            show_LCD(s, &fd);
        }
        // Jump to order list
        else if (key_value == '2'){
            strcpy(s, "2\0");
            show_LCD(s, &fd);
            // Wait for command check by '#'
            while(1){
                // Read key
                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                // ret < 0 means no input
                if (ret < 0){
                    sleep(1);
                    continue;
                }
                ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                unsigned short key_value = (key & 0xff);
                if (key_value == '#'){
                    break;
                }
            }
            strcpy(s, SHOP_LIST2);
            show_LCD(s, &fd);
            // Choose a shop to purchase
            while(1){
                // Read key
                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                // ret < 0 means no input
                if (ret < 0){
                    sleep(1);
                    continue;
                }
                ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                unsigned short key_value = (key & 0xff);
                if (key_value < '1' || key_value > '3'){
                    continue;
                }
                else{
                    unsigned short int shop_idx = key_value - '0' - 1;
                    s[0] = key_value;
                    s[1] = '\0';
                    show_LCD(s, &fd);
                    // Wait for command check by '#'
                    while(1){
                        // Read key
                        ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                        // ret < 0 means no input
                        if (ret < 0){
                            sleep(1);
                            continue;
                        }
                        ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                        unsigned short key_value = (key & 0xff);
                        if (key_value == '#'){
                            break;
                        }
                    }
                    money = 0;  // initial money when choosing a shop
                    char dist = distance[key_value - '0' - 1];
                    int menu_num = key_value - '0' - 1;
                    // Choose what you want to purchase
                    while(1){
                        strcpy(s, menu[menu_num]);
                        show_LCD(s, &fd);
                        // Read key
                        ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                        // ret < 0 means no input
                        if (ret < 0){
                            sleep(1);
                            continue;
                        }
                        ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                        unsigned short key_value = (key & 0xff);
                        if (key_value < '1' || key_value > '4'){
                            continue;
                        }
                        if (key_value == '3'){
                            s[0] = key_value;
                            s[1] = '\0';
                            show_LCD(s, &fd);
                            // Wait for command check by '#'
                            while(1){
                                // Read key
                                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                                // ret < 0 means no input
                                if (ret < 0){
                                    sleep(1);
                                    continue;
                                }
                                ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                                unsigned short key_value = (key & 0xff);
                                if (key_value == '#'){
                                    break;
                                }
                            }
                            // Check if you order anything
                            if (money == 0){
                                printf("Haven't choose any food\n");
                                show_LCD("Haven't choose any food\n", &fd);
                                sleep(2);
                                continue;
                            }
                            else{
                                printf("Total: %d\n", money);
                            }
                            // Send the order and wait
                            show_LCD("Please wait for few minutes...", &fd);
                            // Show how much to pay
                            show_7SEG(money, &fd);
                            int i = 0;
                            for (; i <= dist; i++){
                                show_LED(dist - i, &fd);
                                sleep(1);
                            }
                            show_LCD("please pick up your meal", &fd);
                            // Press any key to go back to START_MENU
                            while(1){
                                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                                // ret < 0 means no input
                                if (ret < 0){
                                    sleep(1);
                                    continue;
                                }
                                else{
                                    break;
                                }
                            }
                            break;
                        }
                        else if (key_value == '4'){
                            // Cancel the order and back to START_MENU
                            // Set money to 0
                            money = 0;
                            show_7SEG(money, &fd);
                            // Go back to START_MENU
                            strcpy(s, START_MENU);
                            show_LCD(s, &fd);
                            break;
                        }
                        else{
                            unsigned short int obj = key_value - '0' - 1;
                            s[0] = key_value;
                            s[1] = '\0';
                            show_LCD(s, &fd);
                            // Wait for command check by '#'
                            while(1){
                                // Read key
                                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                                // ret < 0 means no input
                                if (ret < 0){
                                    sleep(1);
                                    continue;
                                }
                                ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                                unsigned short key_value = (key & 0xff);
                                if (key_value == '#'){
                                    break;
                                }
                            }
                            strcpy(s, "How many\n");
                            show_LCD(s, &fd);
                            // Enter how many you want to buy
                            int num = 0;
                            while(1){
                                // Read key
                                ret = ioctl(fd, KEY_IOCTL_CHECK_EMTPY, &key);
                                // ret < 0 means no input
                                if (ret < 0){
                                    sleep(1);
                                    continue;
                                }
                                ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
                                unsigned short key_value = (key & 0xff);
                                if (key_value == '#'){
                                    // Add money to pay
                                    money += num * cost[shop_idx][obj];
                                    show_7SEG(money, &fd);
                                    // Back to menu
                                    strcpy(s, menu[menu_num]);
                                    show_LCD(s, &fd);
                                    break;
                                }
                                else if (key_value >= '0' && key_value <= '9'){
                                    char value[2];
                                    value[0] = key_value;
                                    value[1] = '\0';
                                    num = 10 * num + key_value - '0';
                                    strcat(s, value);
                                    show_LCD(s, &fd);
                                }
                                else{
                                    continue;
                                }
                            }
                        }
                    }
                    break; // back to START_MENU
                }
            }
        }
        // Other key should not do anything
        else{
            strcpy(s, "Press 1 or 2 to keep going\0");
            show_LCD(s, &fd);
            sleep(2);
            show_LCD(START_MENU, &fd);
        }
    }

    ioctl(fd, _7SEG_IOCTL_OFF, NULL);
    // Close fd
    close(fd);
    return 0;
}
