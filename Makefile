obj-m := connect_lb.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
BUILD_DIR := /lib/modules/`uname -r`/build

connect_lb.ko: connect_lb.c
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean
