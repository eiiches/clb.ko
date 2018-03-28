end-of-list :=

obj-m += connect_lb.o

connect_lb-objs := \
	clb.o \
	clb-api.o \
	clb-member-address.o \
	clb-member.o \
	clb-virtual-server.o \
	clb-virtual-server-address.o \
	module.o \
	module-netlink.o \
	module-netns.o \
	module-syscall-connect.o \
	module-syscall.o \
	module-test.o \
	sockaddr.o \
	$(end-of-list)

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
