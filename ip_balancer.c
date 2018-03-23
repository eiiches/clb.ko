#include <linux/version.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init init_ip_balancer_module(void)
{
	printk(KERN_INFO "ip_balancer: init\n");
	return 0;
}

static void __exit exit_ip_balancer_module(void)
{
	printk(KERN_INFO "ip_balancer: exit\n");
	return;
}

module_init(init_ip_balancer_module);
module_exit(exit_ip_balancer_module);
