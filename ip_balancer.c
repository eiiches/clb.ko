#include <linux/version.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/syscall.h>
#include <asm/errno.h>
#include <asm/pgtable.h>
#include <linux/security.h>
#include <linux/audit.h>
#include <linux/net.h>
#include <linux/file.h>

// REFERENCES:
//  * net/socket.c : sys_connect
//  * fs/file_table.c (include/linux/file.h) : fput
//  * include/linux/net.h : struct socket

MODULE_LICENSE("GPL");

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

typedef long (*connect_fn_ptr_t)(int, struct sockaddr __user *, int);

static sys_call_ptr_t *sys_call_table_ptr;
static connect_fn_ptr_t orig_connect;

#ifdef CONFIG_SECURITY_NETWORK
static int (*security_socket_connect_fn)(struct socket *sock, struct sockaddr *address, int addrlen);
static int security_socket_connect_noop(struct socket *sock, struct sockaddr *address, int addrlen) {
	return 0;
}
#endif


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

	int err;

	struct socket *sock = sockfd_lookup(fd, &err);
	if (!sock)
		goto do_return;

	// Copy address to kernel. For IPv4 or IPv6, addrlen may not be zero.
	struct sockaddr_storage address;
	if (addrlen < 0 || addrlen > sizeof(struct sockaddr_storage)) {
		err = -EINVAL;
		goto fput_and_return;
	}
	if (addrlen != 0) {
		if (copy_from_user(&address, uservaddr, addrlen)) {
			err = -EFAULT;
			goto fput_and_return;
		}
		// audit
		err = audit_sockaddr(addrlen, &address);
		if (err)
			goto fput_and_return;
	}

#ifdef CONFIG_SECURITY_NETWORK
	// security
	err = security_socket_connect_fn(sock, (struct sockaddr *) &address, addrlen);
	if (err)
		goto fput_and_return;
#endif

	// If the socket is neither TCP or UDP, use the original connect(2).
	if (sock->type != SOCK_STREAM && sock->type != SOCK_DGRAM)
		goto connect;
	// If the socket is neither IPv4 or IPv6, use the original connect(2).
	if (sock->ops->family != AF_INET && sock->ops->family != AF_INET6)
		goto connect;

	// TODO: do actual balancing
	pr_info("connect(%d): load balanced", fd);

connect:
	err = sock->ops->connect(sock, (struct sockaddr *) &address, addrlen, sock->file->f_flags);

fput_and_return:
	fput(sock->file);

do_return:
	if (err)
		pr_warn("connect(%d) => %d", fd, err);
	return err;
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

#ifdef CONFIG_SECURITY_NETWORK
	security_socket_connect_fn = (void *) kallsyms_lookup_name("security_socket_connect");
	if (!security_socket_connect_fn) {
		pr_warn("cloud not locate security_socket_connect() address. security features will be disabled.");
		security_socket_connect_fn = security_socket_connect_noop;
	}
#endif

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
