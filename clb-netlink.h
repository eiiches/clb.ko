#ifndef _CLB_NETLINK_H_
#define _CLB_NETLINK_H_


struct sk_buff;

extern void clb_netlink_recv_msg(struct sk_buff *skb);


#endif /* _CLB_NETLINK_H_ */
