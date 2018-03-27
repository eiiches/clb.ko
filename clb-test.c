#define pr_fmt(fmt) KBUILD_MODNAME " (clb-test.c): " fmt

#include <net/net_namespace.h>
#include <uapi/linux/in.h>

#include "clb.h"


void clb_test(void)
{
    pr_info("executing clb_test()\n");
    int err;

    struct clb_t *clb = clb_new(&init_net);

    struct sockaddr_in sin = {
        .sin_family = AF_INET,
        .sin_port = 8080,
        .sin_addr.s_addr = (1 << 24) + 127, // 127.0.0.1
    };
    struct clb_virtual_server_address_t address = {
        .type = SOCK_STREAM,
    };
    memcpy(&address.addr, &sin, sizeof(sin));
    struct clb_virtual_server_config_t config = {
        .method = 0,
    };


    err = clb_virtual_server_create(clb, &address, &config);
    if (err) {
        pr_warn("clb_virtual_server_create() => %d\n", err);
        goto finally;
    }

    err = clb_virtual_server_update(clb, &address, &config);
    if (err) {
        pr_warn("clb_virtual_server_update() => %d\n", err);
        goto finally;
    }

    err = clb_virtual_server_delete(clb, &address);
    if (err) {
        pr_warn("clb_virtual_server_delete() => %d\n", err);
        goto finally;
    }

finally:
    clb_destroy(clb);
    pr_info("leaving clb_test()\n");
}
