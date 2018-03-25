obj-m += connect_lb.o
connect_lb-objs := module.o connect.o clb.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
BUILD_DIR := /lib/modules/`uname -r`/build

.PHONY: modules
moduels:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean

.PHONY: insmod
insmod: modules
	insmod connect_lb.ko

.PHONY: rmmod
rmmod:
	rmmod connect_lb
