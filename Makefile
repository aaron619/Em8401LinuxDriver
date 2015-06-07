ifneq ($(KERNELRELEASE),)
obj-m := realmagic84xx.o
realmagic84xx-objs := realmagicdev.o ioctl.o dma.o registration.o udev.o devfs.o

else
KDIR        := /lib/modules/$(shell uname -r)/build
PWD         := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install
endif


clean:
	-rm -f *.o *.ko .*.cmd *.mod.c *~ 
	-rm -rf .tmp_versions
