#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/cache.h> // __ro_after_init;
#include <net/genetlink.h> // struct genl_family;
#include <net/sock.h> // sock_net;

#include <uapi/clb/netlink.h>
#include "module-netlink.h"


static int clb_netlink_cmd_create_vs(struct sk_buff *skb, struct genl_info *info) {
    pr_info("clb_netlink_cmd_create_vs(net = %px)\n", sock_net(skb->sk));
    return 0;
}


static const struct nla_policy clb_netlink_cmd_policy[NUM_CLB_NETLINK_CMD_ATTRS] = {
    [CLB_NETLINK_CMD_ATTR_VS] = { .type = NLA_NESTED },
};


static const struct genl_ops clb_netlink_ops[] = {
    {
        .cmd = CLB_NETLINK_COMMAND_CREATE_VS,
        .flags = GENL_ADMIN_PERM,
        .policy = clb_netlink_cmd_policy,
        .doit = clb_netlink_cmd_create_vs,
    }
};


static struct genl_family clb_netlink_family __ro_after_init = {
    .hdrsize = 0,
    .name = CLB_NETLINK_NAME,
    .version = CLB_NETLINK_VERSION,
    .maxattr = NUM_CLB_NETLINK_CMD_ATTRS,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = clb_netlink_ops,
    .n_ops = ARRAY_SIZE(clb_netlink_ops),
};


int __init clb_module_netlink_init(void) {
    return genl_register_family(&clb_netlink_family);
}


void clb_module_netlink_exit(void) {
    genl_unregister_family(&clb_netlink_family);
}
