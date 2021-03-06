#ifndef _CLB_H_
#define _CLB_H_


#include <linux/hashtable.h>
#include <net/net_namespace.h> // struct net;
#include <linux/net.h> // enum sock_type;
#include <linux/socket.h> // struct sockaddr_storage;
#include <linux/types.h> // struct hlist_node; struct list_head;
#include <linux/hashtable.h>
#include <net/net_namespace.h> // struct net;

#include "clb-member.h"
#include "clb-virtual-server.h"
#include "clb-virtual-server-address.h"
#include "clb-virtual-server-config.h"


struct clb_t;

#define NETLINK_CLB 78


#ifdef CLB_PRIVATE


struct clb_t {
    struct net *netns; // network namespace
    struct sock *netlink_sock;

#define CLB_VIRTUAL_SERVER_HASH_BITS (7)
    DECLARE_HASHTABLE(virtual_servers, CLB_VIRTUAL_SERVER_HASH_BITS);

    // List running through all the clb in the same hash slot.
    struct hlist_node hlist;
};


extern struct clb_t *clb_new(struct net *netns);

extern void clb_destroy(struct clb_t *clb);

extern struct clb_virtual_server_t *clb_find_virtual_server_by_address(struct clb_t *clb, struct clb_virtual_server_address_t *address);

extern struct clb_virtual_server_t *clb_find_virtual_server_by_address_and_hash(struct clb_t *clb, struct clb_virtual_server_address_t *address, unsigned long precomputed_address_hash);

extern int clb_register_virtual_server(struct clb_t *clb, struct clb_virtual_server_t *vs);

extern int clb_unregister_virtual_server(struct clb_t *clb, struct clb_virtual_server_t *vs);

extern int clb_load_balancer_do_balance(struct clb_t *clb, int type, struct sockaddr_storage *ss);


#endif /* CLB_PRIVATE */


#endif /* _CLB_H_ */
