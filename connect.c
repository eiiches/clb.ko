#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <asm/errno.h>
#include <linux/security.h>
#include <linux/audit.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/file.h>
#include <uapi/linux/in.h>
#include <uapi/linux/in6.h>

#include "connect.h"


static void clb_connect_do_balance_v4(struct sockaddr_in *addr)
{
    pr_info("do_balance_v4: ip = %pISpc\n", addr);
    addr->sin_port = htons(6379);
    addr->sin_addr.s_addr = (1 << 24) + (111 << 16) + (168 << 8) + 192;
    pr_info("do_balance_v4: ip = %pISpc (after)\n", addr);
}


static void clb_connect_do_balance_v6(struct sockaddr_in6 *addr)
{
    pr_info("do_balance_v6: ip = %pISpc\n", addr);
}


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

    // Rewrite address. If the socket is neither IPv4 or IPv6, keep the original address.
    if (sock->ops->family == AF_INET && address.ss_family == AF_INET) {
        clb_connect_do_balance_v4((struct sockaddr_in *) &address);
    } else if (sock->ops->family == AF_INET6 && address.ss_family == AF_INET6) {
        clb_connect_do_balance_v6((struct sockaddr_in6 *) &address);
    } else {
        goto connect;
    }

#ifdef CONFIG_NET_NS
    pr_info("connect(%d): sock->sk->sk_net = %px\n", fd, sock->sk->sk_net.net);
#endif

connect:
    err = sock->ops->connect(sock, (struct sockaddr *) &address, addrlen, sock->file->f_flags);

fput_and_return:
    fput(sock->file);

do_return:
    pr_info("connect(%d) => %d", fd, err);
    return err;
}
