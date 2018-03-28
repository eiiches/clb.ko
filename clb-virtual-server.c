#define pr_fmt(fmt) KBUILD_MODNAME " (" __FILE__ "): " fmt


#include <linux/slab.h> // kzalloc; kfree

#define CLB_PRIVATE
#include "clb-member.h"
#include "clb-virtual-server.h"


void clb_virtual_server_destroy(struct clb_virtual_server_t *vs)
{
    pr_debug("clb_virtual_server_destroy(%px)\n", vs);

    // destroy all the members
    unsigned int bkt;
    struct hlist_node *tmp;
    struct clb_member_t *iter;
    hash_for_each_safe(vs->members, bkt, tmp, iter, members_node) {
        hash_del(&iter->members_node);
        clb_member_destroy(iter);
    }

    // destroy vs itself
    kfree(vs);
}


struct clb_virtual_server_t *clb_virtual_server_new(struct clb_virtual_server_address_t *address, struct clb_virtual_server_config_t *config)
{
    struct clb_virtual_server_t *vs = (struct clb_virtual_server_t *) kzalloc(sizeof(struct clb_virtual_server_t), GFP_KERNEL);
    if (!vs)
        goto out;
    vs->address = *address;
    vs->config = *config;
    hash_init(vs->members);
    vs->members_in_order = RB_ROOT_CACHED;
out:
    pr_debug("clb_virtual_server_new(...) => %px\n", vs);
    return vs;
}
