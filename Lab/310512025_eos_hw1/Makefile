CC=arm-unknown-linux-gnu-gcc
CFLAGS=-o

ARM_LIB_OPT=/opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/
ARM_INC_OPT=/opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/
MICROTIME_INC_OPT=/home/ddfish/EOS/microtime/linux/include/

all:
	${CC} ${CFLAGS} hw1 hw1.c -L ${ARM_LIB_OPT} -I ${ARM_INC_OPT} -I ${MICROTIME_INC_OPT} -lm -Wall
clean:
	${RM} hw1
