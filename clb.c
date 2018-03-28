#define pr_fmt(fmt) KBUILD_MODNAME " (clb.c): " fmt


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


struct clb_virtual_server_t *clb_find_virtual_server_by_address(struct clb_t *clb,
                                                                struct clb_virtual_server_address_t *address)
{
    unsigned long address_hash = clb_virtual_server_address_hash(address);
    return clb_find_virtual_server_by_address_and_hash(clb, address, address_hash);
}


struct clb_virtual_server_t *clb_find_virtual_server_by_address_and_hash(struct clb_t *clb,
                                                                         struct clb_virtual_server_address_t *address,
                                                                         unsigned long precomputed_address_hash)
{
    struct clb_virtual_server_t *iter;
    hash_for_each_possible(clb->virtual_servers, iter, hlist, precomputed_address_hash) {
        if (clb_virtual_server_address_equals(&iter->address, address))
            return iter;
    }
    return NULL;
}


int clb_create_virtual_server(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address,
                              struct clb_virtual_server_config_t *config)
{
    // TODO: validate address and config

    unsigned long address_hash = clb_virtual_server_address_hash(address);
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address_and_hash(clb, address, address_hash);
    if (vs)
        return -EEXIST;

    vs = clb_virtual_server_new(address, config);
    if (!vs)
        return -ENOMEM;
    hash_add(clb->virtual_servers, &vs->hlist, address_hash);
    return 0;
}


int clb_update_virtual_server(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address,
                              struct clb_virtual_server_config_t *config)
{
    // TODO: validate address and config

    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, address);
    if (!vs)
        return -ENOENT;

    vs->config = *config;

    return 0;
}


int clb_delete_virtual_server(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address)
{
    // TODO: validate address and config

    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, address);
    if (!vs)
        return -ENOENT;
    hash_del(&vs->hlist);
    clb_virtual_server_destroy(vs);
    return 0;
}




int clb_virtual_server_add_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *address, struct clb_member_config_t *config)
{
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, server);
    if (!vs)
        return -ENOENT;
    struct clb_member_t *member = clb_member_new(address, config);
    if (!member)
        return -ENOMEM;
    return clb_virtual_server_register_member(vs, member);
}


int clb_virtual_server_change_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *address, struct clb_member_config_t *config)
{
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, server);
    if (!vs)
        return -ENOENT;
    struct clb_member_t *member = clb_virtual_server_find_member_by_address(vs, address);
    if (!member)
        return -ENOENT;
    clb_member_update_config(member, config);
    return 0;
}


int clb_virtual_server_remove_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *address)
{
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, server);
    if (!vs)
        return -ENOENT;
    struct clb_member_t *member = clb_virtual_server_find_member_by_address(vs, address);
    if (!member)
        return -ENOENT;
    return clb_virtual_server_unregister_member(vs, member);
}
