#ifndef _CLB_INTERNAL_H_
#define _CLB_INTERNAL_H_

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


struct clb_virtual_server_t {
	struct clb_virtual_server_address_t address;
	struct clb_virtual_server_config_t config;
};


struct clb_member_t {
	struct clb_member_address_t address;
	struct clb_member_config_t config;
};


#endif /* _CLB_INTERNAL_H_ */