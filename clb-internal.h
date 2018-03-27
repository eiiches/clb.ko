#ifndef _CLB_INTERNAL_H_
#define _CLB_INTERNAL_H_


#include <linux/types.h> // struct hlist_node; struct list_head;
#include <linux/hashtable.h>
#include <net/net_namespace.h> // struct net;


#include "clb.h"


// This struct exsits for each network namespace if CONFIG_NET_NS.
struct clb_t {

#ifdef CONFIG_NET_NS
    const struct net *netns; // network namespace
#endif

#define CLB_VIRTUAL_SERVER_HASH_BITS (7)
    DECLARE_HASHTABLE(virtual_servers, CLB_VIRTUAL_SERVER_HASH_BITS);

    // List running through all the clb in the same hash slot.
    struct hlist_node hlist;
};


extern struct clb_virtual_server_t *clb_find_virtual_server_by_address(struct clb_t *clb,
                                                                       struct clb_virtual_server_address_t *address);
extern struct clb_virtual_server_t *clb_find_virtual_server_by_address_and_hash(struct clb_t *clb,
                                                                                struct clb_virtual_server_address_t *address,
                                                                                unsigned long precomputed_address_hash);


struct clb_virtual_server_t {
    struct clb_virtual_server_address_t address;
    struct clb_virtual_server_config_t config;

    // List running through all the virtual servers in the same hash slot;
    struct hlist_node hlist;

    // List of members which belong to this virtual server
    struct list_head members;
};


extern struct clb_virtual_server_t *clb_virtual_server_new(struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);
extern void clb_virtual_server_destroy(struct clb_virtual_server_t *vs);


struct clb_member_t {
    struct clb_member_address_t address;
    struct clb_member_config_t config;

    // List of all the members of the virtual server.
    struct list_head list;
};


extern struct clb_member_t *clb_member_new(struct clb_member_address_t *address, struct clb_member_config_t *config);
extern void clb_member_destroy(struct clb_member_t *member);


extern unsigned long clb_virtual_server_address_hash(struct clb_virtual_server_address_t *address);
extern bool clb_virtual_server_address_equals(struct clb_virtual_server_address_t *a, struct clb_virtual_server_address_t *b);


#endif /* _CLB_INTERNAL_H_ */
