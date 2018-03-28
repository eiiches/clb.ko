#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#define CLB_PRIVATE
#include "sockaddr.h"
#include "clb-virtual-server-address.h"


unsigned long clb_virtual_server_address_hash(struct clb_virtual_server_address_t *address)
{
    // TODO: add address->type to hash
    return sockaddr_hash(&address->addr);
}


bool clb_virtual_server_address_equals(struct clb_virtual_server_address_t *a, struct clb_virtual_server_address_t *b) {
    if (a->type != b->type)
        return false;
    return sockaddr_equals(&a->addr, &b->addr);
}
