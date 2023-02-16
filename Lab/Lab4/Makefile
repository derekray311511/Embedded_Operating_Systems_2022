CC = arm-linux-gnueabihf-gcc
CXX = arm-linux-gnueabihf-g++
FLAGS = --static
XLNX_DIR = /home/$(USER)/linux-xlnx
obj-m := mydev.o

all: 

	make -C $(XLNX_DIR) M=$(PWD) modules
	$(CXX) -o writer writer.cpp $(FLAGS)
	$(CXX) -o reader reader.cpp $(FLAGS)

clean:
	make -C $(XLNX_DIR) M=$(PWD) clean
	rm -rf writer writer.o
	rm -rf reader reader.o

.PYONY: clean
