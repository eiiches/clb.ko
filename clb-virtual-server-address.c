#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <uapi/linux/in.h> // struct sockaddr_in;
#include <uapi/linux/in6.h> // struct sockaddr_in6;

#define CLB_PRIVATE
#include "sockaddr.h"
#include "clb-virtual-server-address.h"


unsigned long clb_virtual_server_address_hash(struct clb_virtual_server_address_t *address)
{
    // TODO: address->type
    return sockaddr_hash(&address->addr);
}


bool clb_virtual_server_address_equals(struct clb_virtual_server_address_t *a, struct clb_virtual_server_address_t *b) {
    if (a->type != b->type)
        return false;
    return sockaddr_equals(&a->addr, &b->addr);
}
