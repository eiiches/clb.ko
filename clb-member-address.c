#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#define CLB_PRIVATE
#include "sockaddr.h"
#include "clb-member-address.h"


unsigned long clb_member_address_hash(struct clb_member_address_t *member)
{
    return sockaddr_hash(&member->addr);
}


bool clb_member_address_equals(struct clb_member_address_t *a, struct clb_member_address_t *b)
{
    return sockaddr_equals(&a->addr, &b->addr);
}
