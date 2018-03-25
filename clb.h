#ifndef _CLB_H_
#define _CLB_H_

#include <linux/radix-tree.h>

// This struct exsits for each network namespace if CONFIG_NET_NS.
struct clb_t {

#ifdef CONFIG_NET_NS
	const struct net *netns; // network namespace
#endif

	struct radix_tree_root virtual_servers_tcp_v4;
	struct radix_tree_root virtual_servers_udp_v4;

};

extern struct clb_t *clb_new(const struct net *netns);

extern void clb_destroy(struct clb_t *clb);

#endif /* _CLB_H_ */
