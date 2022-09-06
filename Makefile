#CONFIG_MODULE_SIG=n

obj-m				+= chardev-test-module.o
chardev-test-module-objs	:= my-chardev.o

#CC=gcc
KVERSION = $(shell uname -r)
KERNEL_DIR := /lib/modules/$(KVERSION)/build
EXTRA_CFLAGS = -D USE_KFIFO

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
