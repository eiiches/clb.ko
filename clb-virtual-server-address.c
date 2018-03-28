#define pr_fmt(fmt) KBUILD_MODNAME " (" __FILE__ "): " fmt


#include <uapi/linux/in.h> // struct sockaddr_in;
#include <uapi/linux/in6.h> // struct sockaddr_in6;

#define CLB_PRIVATE
#include "clb-virtual-server-address.h"


unsigned long clb_virtual_server_address_hash(struct clb_virtual_server_address_t *address)
{
    // TODO implement
    return 0;
}


bool clb_virtual_server_address_equals(struct clb_virtual_server_address_t *a, struct clb_virtual_server_address_t *b) {
    if (a->type != b->type)
        return false;
    if (a->addr.ss_family != b->addr.ss_family)
        return false;
    switch (a->addr.ss_family) {
        case AF_INET: {
            struct sockaddr_in *a_in = ((struct sockaddr_in *) a);
            struct sockaddr_in *b_in = ((struct sockaddr_in *) b);
            if (a_in->sin_port != b_in->sin_port)
                return false;
            return a_in->sin_addr.s_addr == b_in->sin_addr.s_addr;
        }
        case AF_INET6: {
            struct sockaddr_in6 *a_in6 = ((struct sockaddr_in6 *) a);
            struct sockaddr_in6 *b_in6 = ((struct sockaddr_in6 *) b);
            // TODO: implement
            return false;
        }
        default:
            return false;
    }
}
