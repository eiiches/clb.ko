#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include "module-netns.h"


#include <net/net_namespace.h>

#define CLB_PRIVATE
#include "clb-load-balancer.h"


// TODO: add proper locking to netns_clbs
#define CLB_NETNS_HASH_BITS (7)
static DEFINE_HASHTABLE(netns_clbs, CLB_NETNS_HASH_BITS);


static int __net_init clb_net_init(struct net *netns) {
    pr_info("net_init: %px\n", netns);

    // create corresponding clb and register a mapping
    struct clb_t *clb = clb_new(netns);
    if (!clb)
        return -ENOMEM;
    hash_add(netns_clbs, &clb->hlist, (unsigned long) netns);

    // FIXME: hard code load balancers --------------------

    struct clb_virtual_server_address_t vsaddr;
    vsaddr.type = SOCK_STREAM;
    vsaddr.addr.ss_family = AF_INET;
    ((struct sockaddr_in *) &vsaddr.addr)->sin_port = htons(6379);
    ((struct sockaddr_in *) &vsaddr.addr)->sin_addr.s_addr = (100 << 24) + (0 << 16) + (0 << 8) + 10;

    struct clb_virtual_server_config_t vsconfig = {0};

    struct clb_virtual_server_t *vs = clb_virtual_server_new(&vsaddr, &vsconfig);
    clb_register_virtual_server(clb, vs);

    struct clb_member_address_t redis1addr;
    redis1addr.addr.ss_family = AF_INET;
    ((struct sockaddr_in *) &redis1addr.addr)->sin_port = htons(6379);
    ((struct sockaddr_in *) &redis1addr.addr)->sin_addr.s_addr = (10 << 24) + (0 << 16) + (0 << 8) + 10;
    struct clb_member_config_t redis1config = {0};
    struct clb_member_t *redis1 = clb_member_new(&redis1addr, &redis1config);
    clb_virtual_server_register_member(vs, redis1);

    struct clb_member_address_t redis2addr;
    redis2addr.addr.ss_family = AF_INET;
    ((struct sockaddr_in *) &redis2addr.addr)->sin_port = htons(6379);
    ((struct sockaddr_in *) &redis2addr.addr)->sin_addr.s_addr = (20 << 24) + (0 << 16) + (0 << 8) + 10;
    struct clb_member_config_t redis2config = {0};
    struct clb_member_t *redis2 = clb_member_new(&redis2addr, &redis2config);
    clb_virtual_server_register_member(vs, redis2);

    // FIXME: hard code load balancers --------------------

    return 0;
}


static void __net_exit clb_net_exit(struct net *net) {
    pr_info("net_exit: %px\n", net);

    // destroy corresponding clb
    struct clb_t *clb = clb_module_find(net);
    if (!clb) {
        pr_warn("clb_net_exit: could not find clb entry for netns");
        return;
    }
    hash_del(&clb->hlist);
    clb_destroy(clb);
}


static struct pernet_operations clb_net_ops = {
    .init = clb_net_init,
    .exit = clb_net_exit,
};


int __init clb_module_netns_init(void) {
    // Per netns init/exit handlers. Also invoked for init_net.
    return register_pernet_subsys(&clb_net_ops);
}


void clb_module_netns_exit(void) {
    // Per netns init/exit handlers. Also invoked for init_net.
    unregister_pernet_subsys(&clb_net_ops);
}


struct clb_t *clb_module_find(struct net *netns) {
    struct clb_t *iter;
    hash_for_each_possible(netns_clbs, iter, hlist, (unsigned long) netns) {
        if (iter->netns != netns)
            continue;
        return iter;
    }
    return NULL;
}
