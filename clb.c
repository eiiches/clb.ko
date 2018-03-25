#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/hashtable.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <net/net_namespace.h>

#include "clb.h"
#include "clb-internal.h"


struct clb_t *clb_new(const struct net *netns)
{
	struct clb_t *clb = (struct clb_t *) kzalloc(sizeof(struct clb_t), GFP_KERNEL);
	clb->netns = netns;
	hash_init(clb->virtual_servers);
	return clb;
}


void clb_destroy(struct clb_t *clb)
{
	kfree(clb);
}
