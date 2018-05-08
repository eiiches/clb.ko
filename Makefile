end-of-list :=

obj-m += clb.o

clb-objs := \
	clb-load-balancer.o \
	clb-api.o \
	clb-member-address.o \
	clb-member.o \
	clb-virtual-server.o \
	clb-virtual-server-address.o \
	module.o \
	module-netns.o \
	module-netlink.o \
	module-syscall-connect.o \
	module-syscall.o \
	module-test.o \
	sockaddr.o \
	libprotobuf-c.o \
	netlink.pb-c.o \
	$(end-of-list)

ccflags-y := -DDEBUG -std=gnu99 -Wno-declaration-after-statement -I$(src)/include -I$(src)/include/std-empty

BUILD_DIR := /lib/modules/`uname -r`/build

.PHONY: modules
modules: netlink.pb-c.c netlink.pb-c.h
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean

.PHONY: modules_install
modules_install:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules_install

.PHONY: help
help:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) help

netlink.pb-c.c netlink.pb-c.h: netlink.proto
	protoc --c_out=. netlink.proto
	sed -i 's:<protobuf-c/protobuf-c.h>:"libprotobuf-c.h":g' netlink.pb-c.{c,h}

.PHONY: insmod
insmod: modules
	sudo insmod clb.ko

.PHONY: rmmod
rmmod:
	sudo rmmod clb

# reference: https://www.kernel.org/doc/Documentation/kbuild/modules.txt
