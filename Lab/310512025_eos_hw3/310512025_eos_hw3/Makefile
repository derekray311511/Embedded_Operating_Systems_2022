CC=gcc
CFLAGS=-Wall

all:
	${CC} ${CFLAGS} -c sockop.c
	${CC} ${CFLAGS} -c hw3.c
	${CC} ${CFLAGS} hw3.o sockop.o -o server -lpthread

clean: 
	${RM} server hw3.o sockop.o