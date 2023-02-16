CC=gcc
CFLAGS=-Wall

all:
	${CC} ${CFLAGS} -c server.c
	${CC} ${CFLAGS} -c sockop.c
	${CC} ${CFLAGS} server.o sockop.o -o lab5

clean: 
	${RM} lab5 server.o sockop.o