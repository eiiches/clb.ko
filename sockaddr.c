#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/in.h> // struct sockaddr_in;
#include <linux/in6.h> // struct sockaddr_in6;
#include <linux/socket.h> // struct sockaddr_storage;

#define CLB_PRIVATE
#include "sockaddr.h"


unsigned long sockaddr_hash(struct sockaddr_storage *addr)
{
    return 0;
}

bool sockaddr_equals(struct sockaddr_storage *a, struct sockaddr_storage *b)
{
    if (a->ss_family != b->ss_family)
        return false;
    switch (a->ss_family) {
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
