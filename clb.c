#define pr_fmt(fmt) KBUILD_MODNAME " (" __FILE__ "): " fmt


#include <linux/hashtable.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <net/net_namespace.h>

#define CLB_PRIVATE
#include "clb.h"


struct clb_t *clb_new(const struct net *netns)
{
    struct clb_t *clb = (struct clb_t *) kzalloc(sizeof(struct clb_t), GFP_KERNEL);
    if (!clb)
        goto out;
    clb->netns = netns;
    hash_init(clb->virtual_servers);
out:
    pr_debug("clb_new(...) => %px\n", clb);
    return clb;
}


void clb_destroy(struct clb_t *clb)
{
    pr_debug("clb_destroy(%px)\n", clb);

    // destroy all virtual servers
    unsigned int bkt;
    struct hlist_node *tmp;
    struct clb_virtual_server_t *iter;
    hash_for_each_safe(clb->virtual_servers, bkt, tmp, iter, hlist) {
        hash_del(&iter->hlist);
        clb_virtual_server_destroy(iter);
    }

    // destroy clb itself
    kfree(clb);
}


struct clb_virtual_server_t *clb_find_virtual_server_by_address(struct clb_t *clb, struct clb_virtual_server_address_t *address)
{
    struct clb_virtual_server_t *iter;
    unsigned long address_hash = clb_virtual_server_address_hash(address);
    hash_for_each_possible(clb->virtual_servers, iter, hlist, address_hash) {
        if (clb_virtual_server_address_equals(&iter->address, address))
            return iter;
    }
    return NULL;
}


int clb_register_virtual_server(struct clb_t *clb, struct clb_virtual_server_t *vs)
{
    // TODO: check already registered
    unsigned long address_hash = clb_virtual_server_address_hash(&vs->address);
    hash_add(clb->virtual_servers, &vs->hlist, address_hash);
    return 0;
}


int clb_unregister_virtual_server(struct clb_t *clb, struct clb_virtual_server_t *vs)
{
    hash_del(&vs->hlist);
    return 0;
}
