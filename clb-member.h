#ifndef _CLB_MEMBER_H_
#define _CLB_MEMBER_H_


#include "clb-member-address.h"
#include "clb-member-config.h"


struct clb_member_t;


#ifdef CLB_PRIVATE


#include <linux/hashtable.h>
#include <linux/rbtree.h>


struct clb_member_t {
    struct clb_member_address_t address;
    struct clb_member_config_t config;

    struct hlist_node members_node;

    // Tree node, connected from clb_virtual_server_t#members_in_order.
    struct rb_node members_in_order_node;

    // The number of current connections
    int num_current_connections;

    // The cumulative number of connections attempted. This is used to round-robin connections under connect/close, connect/close, ... kind of situations.
    int num_connection_attempts;
};


extern struct clb_member_t *clb_member_new(struct clb_member_address_t *address, struct clb_member_config_t *config);

extern void clb_member_destroy(struct clb_member_t *member);


#endif /* CLB_PRIVATE */


#endif /* _CLB_MEMBER_H_ */
