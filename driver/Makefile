obj-m += sentinel_driver.o
ccflags-y += -fcf-protection=none -Wa,-mx86-used-note=no

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: default clean
