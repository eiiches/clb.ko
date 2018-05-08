#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <asm/errno.h>
#include <linux/security.h>
#include <linux/audit.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/file.h>
#include <linux/in.h>
#include <linux/in6.h>

#define CLB_PRIVATE
#include "clb-load-balancer.h"
#include "module-syscall-connect.h"
#include "module-netns.h"


#ifdef CONFIG_SECURITY_NETWORK
static int security_socket_connect_noop(struct socket *sock, struct sockaddr *address, int addrlen) {
    return 0;
}
static security_socket_connect_fn_t security_socket_connect_fn = security_socket_connect_noop;

void clb_connect_set_security(security_socket_connect_fn_t fn)
{
    security_socket_connect_fn = fn;
}
#endif


asmlinkage long clb_connect(int fd, struct sockaddr __user *uservaddr, int addrlen)
{
    int err;

    struct socket *sock = sockfd_lookup(fd, &err);
    if (!sock)
        goto do_return;

    // Copy address to kernel. For IPv4 or IPv6, addrlen may not be zero.
    struct sockaddr_storage address;
    if (addrlen < 0 || addrlen > sizeof(struct sockaddr_storage)) {
        err = -EINVAL;
        goto fput_and_return;
    }
    if (addrlen != 0) {
        if (copy_from_user(&address, uservaddr, addrlen)) {
            err = -EFAULT;
            goto fput_and_return;
        }
        // audit
        err = audit_sockaddr(addrlen, &address);
        if (err)
            goto fput_and_return;
    }

#ifdef CONFIG_SECURITY_NETWORK
    // security
    err = security_socket_connect_fn(sock, (struct sockaddr *) &address, addrlen);
    if (err)
        goto fput_and_return;
#endif

    // If the socket is neither TCP or UDP, skip to connect.
    if (sock->type != SOCK_STREAM && sock->type != SOCK_DGRAM)
        goto connect;

    struct clb_t *clb = clb_module_find(sock_net(sock->sk));
    if (unlikely(!clb)) {
        pr_warn("connect(%d): load balancer not found for netns = %px\n", fd, sock_net(sock->sk));
        goto connect;
    }

    // Rewrite address. If the socket is neither IPv4 or IPv6, keep the original address.
    if ((sock->ops->family == AF_INET && address.ss_family == AF_INET)
            || (sock->ops->family == AF_INET6 && address.ss_family == AF_INET6)) {
        pr_info("do_balance_v4: ip = %pISpc\n", &address);
        int err = clb_load_balancer_do_balance(clb, sock->type, &address);
        // TODO: handle errors
        pr_info("do_balance_v4: ip = %pISpc (after)\n", &address);
    } else {
        goto connect;
    }

    pr_info("connect(%d): sock->sk->sk_net = %px\n", fd, sock_net(sock->sk));

connect:
    err = sock->ops->connect(sock, (struct sockaddr *) &address, addrlen, sock->file->f_flags);

fput_and_return:
    fput(sock->file);

do_return:
    pr_info("connect(%d) => %d", fd, err);
    return err;
}
