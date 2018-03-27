#ifndef _CLB_H_
#define _CLB_H_


#include <linux/hashtable.h>
#include <net/net_namespace.h> // struct net;
#include <linux/net.h> // enum sock_type;
#include <linux/socket.h> // struct sockaddr_storage;


struct clb_t;

extern struct clb_t *clb_new(const struct net *netns);
extern void clb_destroy(struct clb_t *clb);



struct clb_virtual_server_address_t {
    enum sock_type type; // SOCK_STREAM or SOCK_DGRAM
    struct sockaddr_storage addr;
};

struct clb_virtual_server_config_t {
    int method; // CLB_POLICY_ROUND_ROBIN or CLB_POLICY_LEAST_CONN
};

extern void clb_virtual_servers(struct clb_t *clb); // TODO: add callback argument
extern int clb_virtual_server_create(struct clb_t *clb, struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);
extern int clb_virtual_server_update(struct clb_t *clb, struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);
extern int clb_virtual_server_delete(struct clb_t *clb, struct clb_virtual_server_address_t *address);



struct clb_member_address_t {
    struct sockaddr_storage addr;
};

struct clb_member_config_t {
    float weight;
};

extern int clb_virtual_server_members(struct clb_t *clb, struct clb_virtual_server_address_t *server); // TODO: add callback argument
extern int clb_virtual_server_add_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_config_t *config);
extern int clb_virtual_server_change_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_config_t *config);
extern int clb_virtual_server_remove_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member);



#endif /* _CLB_H_ */
