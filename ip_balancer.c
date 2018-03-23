#include <linux/version.h>
#include <linux/module.h>
#include <linux/kallsyms.h>

MODULE_LICENSE("GPL");

// void *sys_call_table[];

static int __init ip_balancer_init(void)
{
	unsigned long sys_call_table;
	printk(KERN_INFO "ip_balancer: init\n");
	sys_call_table = kallsyms_lookup_name("sys_call_table");
	printk(KERN_INFO "ip_balancer: located sys_call_table = %px\n", (void *)sys_call_table);
	return 0;
}

static void __exit ip_balancer_exit(void)
{
	printk(KERN_INFO "ip_balancer: exit\n");
	return;
}

module_init(ip_balancer_init);
module_exit(ip_balancer_exit);
