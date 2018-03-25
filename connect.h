#ifndef _CONNECT_H_
#define _CONNECT_H_

#include <linux/net.h>

#ifdef CONFIG_SECURITY_NETWORK

typedef int (*security_socket_connect_fn_t)(struct socket *sock, struct sockaddr *address, int addrlen);

extern void clb_connect_set_security(security_socket_connect_fn_t fn);

#endif

extern asmlinkage long clb_connect(int fd, struct sockaddr __user *uservaddr, int addrlen);

#endif /* _CONNECT_H_ */
