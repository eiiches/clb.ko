obj-m := ip_balancer.o
BUILD_DIR := /lib/modules/`uname -r`/build
PWD := `pwd`
ip_balancer.ko: ip_balancer.c
	make -C $(BUILD_DIR) M=$(PWD) modules
