#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/version.h>

#include "module-test.h"
#include "module-netns.h"
#include "module-syscall.h"
#include "module-netlink.h"


static int __init clb_module_init(void)
{
    pr_info("init\n");

    clb_module_test();

    int err;
    clb_module_netns_init();
    err = clb_module_syscall_init();
    if (err)
        goto fail_syscall_init;
    clb_module_netlink_init();
    return 0;
fail_syscall_init:
    clb_module_netns_exit();
    return err;
}


static void __exit clb_module_exit(void)
{
    clb_module_netlink_exit();
    clb_module_syscall_exit();
    clb_module_netns_exit(); // FIXME: Possible use-after-free. Some clients may still be running our connect(2) handler even after syscall_exit() returned.
    pr_info("exit\n");
    return;
}


module_init(clb_module_init);
module_exit(clb_module_exit);
MODULE_LICENSE("GPL");
