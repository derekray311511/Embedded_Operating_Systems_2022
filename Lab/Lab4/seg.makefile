CC=arm-linux-gnueabihf-gcc

obj-m := segdev.o

all:
	make -C ../ M=$(PWD) modules
clean:
	make -C ../ M=$(PWD) clean
