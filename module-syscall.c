#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


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
#include "module-syscall.h"


typedef long (*connect_fn_ptr_t)(int, struct sockaddr __user *, int);

static sys_call_ptr_t *sys_call_table_ptr;
static connect_fn_ptr_t orig_connect;


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


int clb_module_syscall_init(void) {
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


int clb_module_syscall_exit(void) {
    return clb_override_sys_call(__NR_connect, (sys_call_ptr_t) orig_connect);
}
