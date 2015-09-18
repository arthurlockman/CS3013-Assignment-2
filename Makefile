ccflags-y=-std=gnu99
obj-m := part1.o
obj-m += part2.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	g++ part2test.cpp -o part2test

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm part2test test
