#ifndef _NETNS_H_
#define _NETNS_H_


#include <linux/init.h>


struct net;

struct clb_t;

extern int __init clb_module_netns_init(void);

extern void clb_module_netns_exit(void);

extern struct clb_t *clb_module_find(struct net *netns);


#endif /* _NETNS_H_ */
