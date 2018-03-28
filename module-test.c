#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <net/net_namespace.h>
#include <uapi/linux/in.h>

#define CLB_PRIVATE
#include "clb-api.h"


void clb_module_test(void)
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


    err = clb_create_virtual_server(clb, &address, &config);
    if (err) {
        pr_warn("clb_create_virtual_server() => %d\n", err);
        goto finally;
    }

    struct clb_member_address_t member;
    struct sockaddr_in msin = {
        .sin_family = AF_INET,
        .sin_port = 8080,
        .sin_addr.s_addr = (2 << 24) + 127, // 127.0.0.2
    };
    clb_member_address_set_sockaddr_in(&member, &msin);
    struct clb_member_config_t member_config = {
        .weight = 1.0f
    };

    err = clb_virtual_server_add_member(clb, &address, &member, &member_config);
    if (err) {
        pr_warn("clb_virtual_server_add_member() => %d\n", err);
        goto finally;
    }

    err = clb_update_virtual_server(clb, &address, &config);
    if (err) {
        pr_warn("clb_update_virtual_server() => %d\n", err);
        goto finally;
    }

    err = clb_delete_virtual_server(clb, &address);
    if (err) {
        pr_warn("clb_delete_virtual_server() => %d\n", err);
        goto finally;
    }

finally:
    clb_destroy(clb);
    pr_info("leaving clb_test()\n");
}
