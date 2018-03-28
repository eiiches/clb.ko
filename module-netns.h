#ifndef _NETNS_H_
#define _NETNS_H_


struct net;

struct clb_t;

extern void clb_netns_init(void);

extern void clb_netns_exit(void);

extern struct clb_t *clb_find(struct net *netns);


#endif /* _NETNS_H_ */
