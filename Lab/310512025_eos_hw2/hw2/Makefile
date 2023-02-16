CC=gcc
CFLAGS=-Wall

all:
	${CC} ${CFLAGS} -c sockop.c
	${CC} ${CFLAGS} -c client.c
	${CC} ${CFLAGS} -c hw2.c
	${CC} ${CFLAGS} client.o sockop.o -o client -lpthread
	${CC} ${CFLAGS} hw2.o sockop.o -o server -lpthread

clean: 
	${RM} server client hw2.o sockop.o client.o