#define pr_fmt(fmt) KBUILD_MODNAME " (clb.c): " fmt

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


struct clb_virtual_server_t *clb_virtual_server_get_internal(struct clb_t *clb,
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


void clb_virtual_server_destroy(struct clb_virtual_server_t *vs) {
    pr_debug("clb_virtual_server_destroy(%px)\n", vs);

    // TODO: free members

    kfree(vs);
}


struct clb_virtual_server_t *clb_virtual_server_new(struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config) {
    struct clb_virtual_server_t *vs = (struct clb_virtual_server_t *) kzalloc(sizeof(struct clb_virtual_server_t), GFP_KERNEL);
    vs->address = *address;
    vs->config = *config;
    INIT_LIST_HEAD(&vs->members);
    pr_debug("clb_virtual_server_new(...) => %px\n", vs);
    return vs;
}


int clb_virtual_server_create(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address,
                              struct clb_virtual_server_config_t *config)
{
    // TODO: validate address and config

    unsigned long address_hash = clb_virtual_server_address_hash(address);
    struct clb_virtual_server_t *vs = clb_virtual_server_get_internal(clb, address, address_hash);
    if (vs)
        return -EEXIST;

    vs = clb_virtual_server_new(address, config);
    hash_add(clb->virtual_servers, &vs->hlist, address_hash);
    return 0;
}


int clb_virtual_server_update(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address,
                              struct clb_virtual_server_config_t *config)
{
    // TODO: validate address and config

    unsigned long address_hash = clb_virtual_server_address_hash(address);
    struct clb_virtual_server_t *vs = clb_virtual_server_get_internal(clb, address, address_hash);
    if (!vs)
        return -ENOENT;

    vs->config = *config;

    return 0;
}


int clb_virtual_server_delete(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address)
{
    // TODO: validate address and config

    unsigned long address_hash = clb_virtual_server_address_hash(address);
    struct clb_virtual_server_t *vs = clb_virtual_server_get_internal(clb, address, address_hash);
    if (!vs)
        return -ENOENT;
    hash_del(&vs->hlist);
    clb_virtual_server_destroy(vs);
    return 0;
}


unsigned long clb_virtual_server_address_hash(struct clb_virtual_server_address_t *address)
{
    // TODO implement
    return 0;
}


bool clb_virtual_server_address_equals(struct clb_virtual_server_address_t *a, struct clb_virtual_server_address_t *b) {
    if (a->type != b->type)
        return false;
    if (a->addr.ss_family != b->addr.ss_family)
        return false;
    switch (a->addr.ss_family) {
        case AF_INET: {
            struct sockaddr_in *a_in = ((struct sockaddr_in *) a);
            struct sockaddr_in *b_in = ((struct sockaddr_in *) b);
            if (a_in->sin_port != b_in->sin_port)
                return false;
            return a_in->sin_addr.s_addr == b_in->sin_addr.s_addr;
        }
        case AF_INET6: {
            struct sockaddr_in6 *a_in6 = ((struct sockaddr_in6 *) a);
            struct sockaddr_in6 *b_in6 = ((struct sockaddr_in6 *) b);
            // TODO: implement
            return false;
        }
        default:
            return false;
    }
}
