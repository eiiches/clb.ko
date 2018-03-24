obj-m := sock_vs.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
BUILD_DIR := /lib/modules/`uname -r`/build

sock_vs.ko: sock_vs.c
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean
