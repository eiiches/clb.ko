#ifndef _CLB_VIRTUAL_SERVER_ADDRESS_H_
#define _CLB_VIRTUAL_SERVER_ADDRESS_H_


#include <linux/net.h> // enum sock_type;
#include <linux/socket.h> // struct sockaddr_storage


struct clb_virtual_server_address_t {
    enum sock_type type; // SOCK_STREAM or SOCK_DGRAM
    struct sockaddr_storage addr;
};


#ifdef CLB_PRIVATE


extern unsigned long clb_virtual_server_address_hash(struct clb_virtual_server_address_t *address);

extern bool clb_virtual_server_address_equals(struct clb_virtual_server_address_t *a, struct clb_virtual_server_address_t *b);


#endif /* CLB_PRIVATE */


#endif /* _CLB_VIRTUAL_SERVER_ADDRESS_H_ */
