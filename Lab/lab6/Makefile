CC=gcc
CFLAGS=-Wall

all:
	${CC} ${CFLAGS} -c sockop.c
	${CC} ${CFLAGS} -c client.c
	${CC} ${CFLAGS} -c Web_ATM.c
	${CC} ${CFLAGS} client.o sockop.o -o client -lpthread
	${CC} ${CFLAGS} Web_ATM.o sockop.o -o server -lpthread

clean: 
	${RM} server client Web_ATM.o sockop.o client.o