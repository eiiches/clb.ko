#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

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
#include <net/sock.h>
#include <linux/file.h>
#include <uapi/linux/in.h>
#include <uapi/linux/in6.h>

// REFERENCES:
//  * net/socket.c : sys_connect
//  * fs/file_table.c (include/linux/file.h) : fput
//  * include/linux/net.h : struct socket

MODULE_LICENSE("GPL");

typedef long (*connect_fn_ptr_t)(int, struct sockaddr __user *, int);

static sys_call_ptr_t *sys_call_table_ptr;
static connect_fn_ptr_t orig_connect;

#ifdef CONFIG_SECURITY_NETWORK
static int (*security_socket_connect_fn)(struct socket *sock, struct sockaddr *address, int addrlen);
static int security_socket_connect_noop(struct socket *sock, struct sockaddr *address, int addrlen) {
	return 0;
}
#endif


static int clb_override_sys_call(int sys_call_nr, sys_call_ptr_t sys_call_fn) {
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


static void clb_do_balance_v4(struct sockaddr_in *addr)
{
	pr_info("do_balance_v4: ip = %pISpc\n", addr);
	addr->sin_port = htons(6379);
	addr->sin_addr.s_addr = (1 << 24) + (111 << 16) + (168 << 8) + 192;
	pr_info("do_balance_v4: ip = %pISpc (after)\n", addr);
}


static void clb_do_balance_v6(struct sockaddr_in6 *addr)
{
	pr_info("do_balance_v6: ip = %pISpc\n", addr);
}


asmlinkage long clb_connect(int fd, struct sockaddr __user *uservaddr, int addrlen)
{
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

	// If the socket is neither TCP or UDP, skip to connect.
	if (sock->type != SOCK_STREAM && sock->type != SOCK_DGRAM)
		goto connect;

	// Rewrite address. If the socket is neither IPv4 or IPv6, keep the original address.
	if (sock->ops->family == AF_INET && address.ss_family == AF_INET) {
		clb_do_balance_v4((struct sockaddr_in *) &address);
	} else if (sock->ops->family == AF_INET6 && address.ss_family == AF_INET6) {
		clb_do_balance_v6((struct sockaddr_in6 *) &address);
	} else {
		goto connect;
	}

#ifdef CONFIG_NET_NS
	pr_info("connect(%d): sock->sk->sk_net = %px\n", fd, sock->sk->sk_net.net);
#endif

connect:
	err = sock->ops->connect(sock, (struct sockaddr *) &address, addrlen, sock->file->f_flags);

fput_and_return:
	fput(sock->file);

do_return:
	pr_info("connect(%d) => %d", fd, err);
	return err;
}

static int __net_init clb_net_init(struct net *net) {
	pr_info("net_init: %px\n", net);
	return 0;
}

static void __net_exit clb_net_exit(struct net *net) {
	pr_info("net_exit: %px\n", net);
}

static struct pernet_operations clb_net_ops = {
	.init = clb_net_init,
	.exit = clb_net_exit,
};

static int __init clb_init(void)
{
	pr_info("init\n");

#ifdef CONFIG_NET_NS
	// Per netns init/exit handlers. Also invoked for init_net.
	register_pernet_subsys(&clb_net_ops);
#endif

	sys_call_table_ptr = (sys_call_ptr_t *) kallsyms_lookup_name("sys_call_table");
	if (!sys_call_table_ptr) {
		pr_err("could not locate sys_call_table, exiting.");
		return -EFAULT;
	}
	pr_info("linked sys_call_table => %px\n", sys_call_table_ptr);

	orig_connect = (connect_fn_ptr_t) sys_call_table_ptr[__NR_connect];
	connect_fn_ptr_t orig_connect_check = (connect_fn_ptr_t) kallsyms_lookup_name("sys_connect");
	if (orig_connect != orig_connect_check) {
		pr_err("could not validate connect(2) address, exiting:");
		pr_err(" * sys_call_table[__NR_connect] => %px\n", orig_connect);
		pr_err(" * /proc/kallsyms:sys_connect => %px\n", orig_connect_check);
		return -EFAULT;
	}
	pr_info("linked connect(2) => %px\n", orig_connect);

#ifdef CONFIG_SECURITY_NETWORK
	security_socket_connect_fn = (void *) kallsyms_lookup_name("security_socket_connect");
	if (!security_socket_connect_fn) {
		pr_warn("cloud not locate security_socket_connect() address. security features will be disabled.");
		security_socket_connect_fn = security_socket_connect_noop;
	} else {
		pr_info("linked security_socket_connect() => %px\n", security_socket_connect_fn);
	}
#endif

	return clb_override_sys_call(__NR_connect, (sys_call_ptr_t) clb_connect);
}


static void __exit clb_exit(void)
{
	int err = clb_override_sys_call(__NR_connect, (sys_call_ptr_t) orig_connect);
	WARN_ON(err);

#ifdef CONFIG_NET_NS
	// Per netns init/exit handlers. Also invoked for init_net.
	unregister_pernet_subsys(&clb_net_ops);
#endif

	pr_info("exit\n");
	return;
}

module_init(clb_init);
module_exit(clb_exit);
