#ifndef _CLB_MEMBER_ADDRESS_H_
#define _CLB_MEMBER_ADDRESS_H_


#include <linux/socket.h> // struct sockaddr_storage;
#include <linux/in.h> // struct sockaddr_in;
#include <linux/in6.h> // struct sockaddr_in6;


struct clb_member_address_t {
    struct sockaddr_storage addr;
};


static inline void clb_member_address_set_sockaddr_in(struct clb_member_address_t *member, struct sockaddr_in *sin) {
    memcpy(&member->addr, sin, sizeof(*sin));
}


static inline void clb_member_address_set_sockaddr_in6(struct clb_member_address_t *member, struct sockaddr_in6 *sin6) {
    memcpy(&member->addr, sin6, sizeof(*sin6));
}


#ifdef CLB_PRIVATE


extern unsigned long clb_member_address_hash(struct clb_member_address_t *member);

extern bool clb_member_address_equals(struct clb_member_address_t *a, struct clb_member_address_t *b);


#endif /* CLB_PRIVATE */


#endif /* _CLB_MEMBER_ADDRESS_H_ */
