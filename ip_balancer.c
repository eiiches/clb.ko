#include <linux/version.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/syscall.h>
#include <asm/errno.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>

MODULE_LICENSE("GPL");

typedef long (*connect_fn_ptr_t)(int, struct sockaddr __user *, int);

static sys_call_ptr_t *sys_call_table_ptr;
static connect_fn_ptr_t orig_connect;


static int ip_balancer_set_connect_sys_call(sys_call_ptr_t connect_fn) {
	unsigned int level;
	pte_t *ptep = lookup_address((unsigned long) sys_call_table_ptr, &level);
	if (unlikely(!ptep))
		return -EFAULT;
	if (pte_none(*ptep))
		return -EFAULT;
	pte_t pte = *ptep;

	const int is_writable = pte_write(pte);
	if (!is_writable) {
		set_pte(ptep, pte_mkwrite(pte));
		// TODO: minimize flush range
		flush_tlb_all();
	}

	sys_call_table_ptr[__NR_connect] = connect_fn;

	if (!is_writable) {
		set_pte(ptep, pte_wrprotect(pte));
		// TODO: minimize flush range
		flush_tlb_all();
	}

	return 0;
}


asmlinkage long ip_balancer_connect(int fd, struct sockaddr __user *uservaddr, int addrlen)
{
	printk(KERN_INFO "ip_balancer: connect(%d)\n", fd);
	return orig_connect(fd, uservaddr, addrlen);
}


static int __init ip_balancer_init(void)
{
	printk(KERN_INFO "ip_balancer: init\n");

	sys_call_table_ptr = (sys_call_ptr_t *) kallsyms_lookup_name("sys_call_table");
	if (!sys_call_table_ptr) {
		printk(KERN_ERR "ip_balancer: could not locate sys_call_table, exiting.");
		return -EFAULT;
	}
	printk(KERN_INFO "ip_balancer: located sys_call_table = %px\n", sys_call_table_ptr);

	orig_connect = (connect_fn_ptr_t) sys_call_table_ptr[__NR_connect];
	connect_fn_ptr_t orig_connect_check = (connect_fn_ptr_t) kallsyms_lookup_name("sys_connect");
	if (orig_connect != orig_connect_check) {
		printk(KERN_ERR "ip_balancer: could not validate connect(2) address, exiting:");
		printk(KERN_ERR "ip_balancer:  * sys_call_table[__NR_connect] == %px\n", orig_connect);
		printk(KERN_ERR "ip_balancer:  * /proc/kallsyms:sys_connect == %px\n", orig_connect_check);
		return -EFAULT;
	}
	printk(KERN_INFO "ip_balancer: located connect(2) = %px\n", orig_connect);

	ip_balancer_set_connect_sys_call((sys_call_ptr_t) ip_balancer_connect);

	return 0;
}


static void __exit ip_balancer_exit(void)
{
	ip_balancer_set_connect_sys_call((sys_call_ptr_t) orig_connect);
	printk(KERN_INFO "ip_balancer: exit\n");
	return;
}

module_init(ip_balancer_init);
module_exit(ip_balancer_exit);
