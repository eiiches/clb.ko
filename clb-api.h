#ifndef _CLB_API_H_
#define _CLB_API_H_


#include "clb-load-balancer.h"


extern int clb_create_virtual_server(struct clb_t *clb, struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);

extern int clb_update_virtual_server(struct clb_t *clb, struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config);

extern int clb_delete_virtual_server(struct clb_t *clb, struct clb_virtual_server_address_t *address);

extern int clb_virtual_server_add_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member, struct clb_member_config_t *config);

extern int clb_virtual_server_change_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member, struct clb_member_config_t *config);

extern int clb_virtual_server_remove_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *member);


#endif /* _CLB_API_H_ */
