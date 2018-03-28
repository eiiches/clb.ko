obj-m += connect_lb.o
connect_lb-objs := module.o connect.o clb.o clb-test.o clb-member-address.o clb-member.o clb-virtual-server.o clb-virtual-server-address.o clb-api.o sockaddr.o module-netns.o
ccflags-y := -DDEBUG -std=gnu99 -Wno-declaration-after-statement
BUILD_DIR := /lib/modules/`uname -r`/build

.PHONY: modules
modules:
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
