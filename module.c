#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/version.h>

#include "module-test.h"
#include "module-netns.h"
#include "module-syscall.h"


static int __init clb_init(void)
{
    pr_info("init\n");
    clb_module_test();
    clb_module_netns_init();
    return clb_module_syscall_init();
}


static void __exit clb_exit(void)
{
    clb_module_syscall_exit();
    clb_module_netns_exit(); // FIXME: Possible use-after-free. Some clients may still be running our connect(2) handler even after syscall_exit() returned.
    pr_info("exit\n");
    return;
}


module_init(clb_init);
module_exit(clb_exit);
MODULE_LICENSE("GPL");
