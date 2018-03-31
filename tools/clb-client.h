#ifndef _CLB_CLIENT_H_
#define _CLB_CLIENT_H_


struct clb_client_t;

extern struct clb_client_t *clb_client_new(void);

extern int clb_client_connect(struct clb_client_t *client);

extern void clb_client_destroy(struct clb_client_t *client);

extern int clb_client_create_virtual_server(struct clb_client_t *client);

extern int clb_client_update_virtual_server(struct clb_client_t *client);

extern int clb_client_delete_virtual_server(struct clb_client_t *client);


#endif /* _CLB_CLIENT_H_ */
