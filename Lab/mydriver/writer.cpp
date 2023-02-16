#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int seg_for_c[27] = {
    0b1111001100010001, // A
    0b0000011100000101, // b
    0b1100111100000000, // C
    0b0000011001000101, // d
    0b1000011100000001, // E
    0b1000001100000001, // F
    0b1001111100010000, // G
    0b0011001100010001, // H
    0b1100110001000100, // I
    0b1100010001000100, // J
    0b0000000001101100, // K
    0b0000111100000000, // L
    0b0011001110100000, // M
    0b0011001110001000, // N
    0b1111111100000000, // O
    0b1000001101000001, // P
    0b0111000001010000, //q
    0b1110001100011001, //R
    0b1101110100010001, //S
    0b1100000001000100, //T
    0b0011111100000000, //U
    0b0000001100100010, //V
    0b0011001100001010, //W
    0b0000000010101010, //X
    0b0000000010100100, //Y
    0b1100110000100010, //Z
    0b0000000000000000
};

int main(int argc, char** argv){
	int fd, data;
	int count = 0;
	char buffer[16];

	if (argc != 2){
		perror("No text provided");
		exit(EXIT_FAILURE);
	}

	if ((fd = open("/dev/mydev", 0_RDWR)) < 0){
		perror("Fail to open mydev\n");
		exit(EXIT_FAILURE);
	}

	// initial buffer
	for (int i = 0; i < 16; i++){
		buffer[i] = 0;
	}
	write(fd, buffer, 16);
	sleep(1);
	printf("write to %s\n", argv[1]);

	for (count = 0; argv[1][count] != '\0'; count++){
		printf("showing %c\n", argv[1][count]);
		data = seg_for_c[argv[1][count] - 'A'];
		for (int i = 0; i < 16; i++){
			buffer[15-i] = (data >> i) % 2;
		}
		write(fd, buffer, 16);
		sleep(1);
	}

	close(fd);
}
