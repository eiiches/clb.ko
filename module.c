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

#include "connect.h"
#include "clb.h"
#include "clb-internal.h"
#include "clb-test.h"

// REFERENCES:
//  * net/socket.c : sys_connect
//  * fs/file_table.c (include/linux/file.h) : fput
//  * include/linux/net.h : struct socket

typedef long (*connect_fn_ptr_t)(int, struct sockaddr __user *, int);

static sys_call_ptr_t *sys_call_table_ptr;
static connect_fn_ptr_t orig_connect;

// TODO: add proper locking to netns_clbs
#define CLB_NETNS_HASH_BITS (7)
static DEFINE_HASHTABLE(netns_clbs, CLB_NETNS_HASH_BITS);

static struct clb_t *clb_find_by_netns(struct net *netns) {
    struct clb_t *iter;
    hash_for_each_possible(netns_clbs, iter, hlist, (unsigned long) netns) {
        if (iter->netns != netns)
            continue;
        return iter;
    }
    return NULL;
}

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


static int __net_init clb_net_init(struct net *netns) {
    pr_info("net_init: %px\n", netns);

    // create corresponding clb and register a mapping
    struct clb_t *clb = clb_new(netns);
    if (!clb)
        return -ENOMEM;

    hash_add(netns_clbs, &clb->hlist, (unsigned long) netns);
    return 0;
}

static void __net_exit clb_net_exit(struct net *net) {
    pr_info("net_exit: %px\n", net);

    // destroy corresponding clb
    struct clb_t *clb = clb_find_by_netns(net);
    if (!clb) {
        pr_warn("clb_net_exit: could not find clb entry for netns");
        return;
    }
    hash_del(&clb->hlist);
    clb_destroy(clb);
}

static struct pernet_operations clb_net_ops = {
    .init = clb_net_init,
    .exit = clb_net_exit,
};

static int __init clb_init(void)
{
    pr_info("init\n");
    clb_test();

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
    security_socket_connect_fn_t security_socket_connect_fn = (void *) kallsyms_lookup_name("security_socket_connect");
    if (!security_socket_connect_fn) {
        pr_warn("cloud not locate security_socket_connect() address. security features will be disabled.");
    } else {
        clb_connect_set_security(security_socket_connect_fn);
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
MODULE_LICENSE("GPL");
