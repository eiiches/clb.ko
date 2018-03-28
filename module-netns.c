#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include "module-netns.h"


#ifdef CONFIG_NET_NS


#include <net/net_namespace.h>

#define CLB_PRIVATE
#include "clb.h"


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


void clb_module_netns_init(void) {
    // Per netns init/exit handlers. Also invoked for init_net.
    register_pernet_subsys(&clb_net_ops);
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


#else /* CONFIG_NET_NS */


static struct clb_t *clb;


void clb_module_netns_init(void) {
    clb = clb_new(NULL);
}

void clb_module_netns_exit(void) {
    clb_destroy(clb);
}

struct clb_t *clb_module_find(struct net *netns) {
    return clb;
}


#endif /* CONFIG_NET_NS */
