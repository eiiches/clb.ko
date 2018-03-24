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

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

typedef long (*connect_fn_ptr_t)(int, struct sockaddr __user *, int);

static sys_call_ptr_t *sys_call_table_ptr;
static connect_fn_ptr_t orig_connect;


static int ip_balancer_override_sys_call(int sys_call_nr, sys_call_ptr_t sys_call_fn) {
	unsigned int level;
	pte_t *ptep = lookup_address((unsigned long) sys_call_table_ptr, &level);
	if (unlikely(!ptep))
		return -EFAULT;
	if (unlikely(pte_none(*ptep)))
		return -EFAULT;
	pte_t pte = *ptep;

	const int is_writable = pte_write(pte);
	if (!is_writable) {
		set_pte(ptep, pte_mkwrite(pte));
	}

	sys_call_table_ptr[sys_call_nr] = sys_call_fn;

	if (!is_writable) {
		set_pte(ptep, pte_wrprotect(pte));
	}

	return 0;
}


asmlinkage long ip_balancer_connect(int fd, struct sockaddr __user *uservaddr, int addrlen)
{
	pr_info("connect(%d)\n", fd);
	return orig_connect(fd, uservaddr, addrlen);
}


static int __init ip_balancer_init(void)
{
	pr_info("init\n");

	sys_call_table_ptr = (sys_call_ptr_t *) kallsyms_lookup_name("sys_call_table");
	if (!sys_call_table_ptr) {
		pr_err("could not locate sys_call_table, exiting.");
		return -EFAULT;
	}
	pr_info("located sys_call_table = %px\n", sys_call_table_ptr);

	orig_connect = (connect_fn_ptr_t) sys_call_table_ptr[__NR_connect];
	connect_fn_ptr_t orig_connect_check = (connect_fn_ptr_t) kallsyms_lookup_name("sys_connect");
	if (orig_connect != orig_connect_check) {
		pr_err("could not validate connect(2) address, exiting:");
		pr_err(" * sys_call_table[__NR_connect] == %px\n", orig_connect);
		pr_err(" * /proc/kallsyms:sys_connect == %px\n", orig_connect_check);
		return -EFAULT;
	}
	pr_info("located connect(2) = %px\n", orig_connect);

	return ip_balancer_override_sys_call(__NR_connect, (sys_call_ptr_t) ip_balancer_connect);
}


static void __exit ip_balancer_exit(void)
{
	int err = ip_balancer_override_sys_call(__NR_connect, (sys_call_ptr_t) orig_connect);
	WARN_ON(err);

	pr_info("exit\n");
	return;
}

module_init(ip_balancer_init);
module_exit(ip_balancer_exit);
