#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include <linux/net.h>

struct clb_virtual_server_t {
	enum sock_type type; // SOCK_STREAM or SOCK_DGRAM
	struct sockaddr_storage addr;
};

struct clb_virtual_server_config_t {
	struct clb_virtual_server_t server;
	int method; // CLB_POLICY_ROUND_ROBIN or CLB_POLICY_LEAST_CONN
};

struct clb_real_server_t {
	struct sockaddr_storage addr;
};

struct clb_real_server_config_t {
	struct clb_real_server_t server;
	float weight;
};

// --- functions for virtual servers

// list all virtual servers
extern void clb_virtual_servers(void);

// -EEXISTS if already exist
extern void clb_create_virtual_server(struct clb_virtual_server_config_t *config);

// -ENOEXIST if not exist
extern void clb_update_virtual_server(struct clb_virtual_server_config_t *config);

// -ENOEXIST if not exist
extern void clb_delete_virtual_server(struct clb_virtual_server_t *server);

// --- functions for real servers

// list all real servers of the specified virtual server
extern void clb_virtual_server_real_servers(struct clb_virtual_server_t *server);

// add real server to the specified virtual server
extern void clb_virtual_server_add_real_server(struct clb_virtual_server_t *server,
											   struct clb_real_server_config_t *config);

// change the attribute of the specified real server such as weight, etc.
extern void clb_virtual_server_change_real_server(struct clb_virtual_server_t *server,
												  struct clb_real_server_config_t *config);

// remove a real server from the virtual server
extern void clb_virtual_server_remove_real_server(struct clb_virtual_server_t *server,
												  struct clb_real_server_t *member);

#endif /* _REGISTRY_H_ */
