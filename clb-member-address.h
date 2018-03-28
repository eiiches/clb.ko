#ifndef _CLB_MEMBER_ADDRESS_H_
#define _CLB_MEMBER_ADDRESS_H_


#include <linux/socket.h> // struct sockaddr_storage;


struct clb_member_address_t {
    struct sockaddr_storage addr;
};


#ifdef CLB_PRIVATE


extern unsigned long clb_member_address_hash(struct clb_member_address_t *member);

extern bool clb_member_address_equals(struct clb_member_address_t *a, struct clb_member_address_t *b);


#endif /* CLB_PRIVATE */


#endif /* _CLB_MEMBER_ADDRESS_H_ */
