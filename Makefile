obj-m := ip_balancer.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
BUILD_DIR := /lib/modules/`uname -r`/build

ip_balancer.ko: ip_balancer.c
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean
