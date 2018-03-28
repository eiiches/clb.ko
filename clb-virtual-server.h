#ifndef _CLB_VIRTUAL_SERVER_H_
#define _CLB_VIRTUAL_SERVER_H_


#ifdef CLB_PRIVATE


#include <linux/rbtree.h>
#include <linux/hashtable.h>

#include "clb-virtual-server-address.h"
#include "clb-virtual-server-config.h"


struct clb_virtual_server_t {
    struct clb_virtual_server_address_t address;
    struct clb_virtual_server_config_t config;

    // List running through all the virtual servers in the same hash slot;
    struct hlist_node hlist;

#define CLB_MEMBER_HASH_BITS (7)
    DECLARE_HASHTABLE(members, CLB_MEMBER_HASH_BITS);

    // Tree of members. The left-most node will be used next.
    struct rb_root_cached members_in_order;
};


extern struct clb_virtual_server_t *clb_virtual_server_new(struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);

extern void clb_virtual_server_destroy(struct clb_virtual_server_t *vs);

extern int clb_virtual_server_register_member(struct clb_virtual_server_t *vs, struct clb_member_t *member);

extern int clb_virtual_server_unregister_member(struct clb_virtual_server_t *vs, struct clb_member_t *member);

extern struct clb_member_t *clb_virtual_server_find_member_by_address(struct clb_virtual_server_t *vs, struct clb_member_address_t *address);

extern int clb_virtual_server_update_config(struct clb_virtual_server_t *vs, struct clb_virtual_server_config_t *config);


#endif /* CLB_PRIVATE */


#endif /* _CLB_VIRTUAL_SERVER_H_ */
