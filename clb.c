#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/radix-tree.h>
#include <linux/gfp.h>
#include <net/net_namespace.h>

#include "clb.h"

struct clb_t *clb_new(const struct net *netns)
{
	struct clb_t *clb = (struct clb_t *) kmalloc(sizeof(struct clb_t), GFP_KERNEL);
	clb->netns = netns;
	INIT_RADIX_TREE(&clb->virtual_servers_tcp_v4, GFP_KERNEL);
	INIT_RADIX_TREE(&clb->virtual_servers_udp_v4, GFP_KERNEL);
	return clb;
}

void clb_destroy(struct clb_t *clb)
{
	kfree(clb);
}
