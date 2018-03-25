#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/radix-tree.h>
#include <linux/gfp.h>
#include "registry.h"

MODULE_LICENSE("GPL");

// Radix-tree to map IPv4 virtual server address to virtual_server_t struct.
RADIX_TREE(clb_registry_tcp_v4, GFP_KERNEL);

void clb_create_virtual_server(struct clb_virtual_server_config_t *config) {

}
EXPORT_SYMBOL(clb_create_virtual_server);
