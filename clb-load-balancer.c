#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/hashtable.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <net/net_namespace.h>
#include <linux/netlink.h> // kernel_netlink_create; kernel_netlink_release

#define CLB_PRIVATE
#include "clb-load-balancer.h"


struct clb_t *clb_new(struct net *netns)
{
    struct clb_t *clb = (struct clb_t *) kzalloc(sizeof(struct clb_t), GFP_KERNEL);
    if (!clb)
        goto out;
    clb->netns = netns;
    hash_init(clb->virtual_servers);
out:
    pr_debug("clb_new(%px) => %px\n", netns, clb);
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
    if (clb_virtual_server_is_inuse(vs))
        return -EBUSY; // the virtual server is already registered somewhere
    unsigned long address_hash = clb_virtual_server_address_hash(&vs->address);
    hash_add(clb->virtual_servers, &vs->hlist, address_hash);
    return 0;
}


int clb_unregister_virtual_server(struct clb_t *clb, struct clb_virtual_server_t *vs)
{
    hash_del(&vs->hlist);
    return 0;
}


int clb_load_balancer_do_balance(struct clb_t *clb, int type, struct sockaddr_storage *ss)
{
    int ret = 0;

    struct clb_virtual_server_address_t address;
    address.addr = *ss;
    address.type = type;

    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, &address);
    if (!vs) {
        pr_info("no virtual server found for address %pISpc\n", ss);
        goto ret;
    }

    unsigned long flags;
    spin_lock_irqsave(&vs->next_rr_lock, flags);

    struct clb_member_t *member = vs->next_rr;
    if (!member) {
        member = rb_entry_safe(rb_first_cached(&vs->members_in_order), struct clb_member_t, members_in_order_node);
        if (!member) { // no members
            pr_warn("no members for address %pISpc\n", ss);
            ret = -ECONNRESET;
            goto ret_unlock;
        }
    }
    vs->next_rr = rb_entry_safe(rb_next(&member->members_in_order_node), struct clb_member_t, members_in_order_node);

    *ss = member->address.addr;

ret_unlock:
    spin_unlock_irqrestore(&vs->next_rr_lock, flags);

ret:
    return ret;
}
