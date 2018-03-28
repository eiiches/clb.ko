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

extern struct clb_t *clb_new(const struct net *netns);
extern void clb_destroy(struct clb_t *clb);

extern void clb_virtual_servers(struct clb_t *clb); // TODO: add callback argument
extern int clb_create_virtual_server(struct clb_t *clb, struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);
extern int clb_update_virtual_server(struct clb_t *clb, struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);
extern int clb_delete_virtual_server(struct clb_t *clb, struct clb_virtual_server_address_t *address);


extern int clb_virtual_server_members(struct clb_t *clb, struct clb_virtual_server_address_t *server); // TODO: add callback argument
extern int clb_virtual_server_add_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member, struct clb_member_config_t *config);
extern int clb_virtual_server_change_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member, struct clb_member_config_t *config);
extern int clb_virtual_server_remove_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member);


#ifdef CLB_PRIVATE


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


#endif /* CLB_PRIVATE */


#endif /* _CLB_H_ */
