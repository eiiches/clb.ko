#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


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


int clb_virtual_server_register_member(struct clb_virtual_server_t *vs, struct clb_member_t *member)
{
    if (clb_member_is_inuse(member))
        return -EBUSY; // member is already inuse

    // reset counters
    member->num_current_connections = 0;
    member->num_connection_attempts = 0;

    unsigned long address_hash = clb_member_address_hash(&member->address);
    hash_add(vs->members, &member->members_node, address_hash);

    struct rb_node **link = &vs->members_in_order.rb_root.rb_node;
    struct rb_node *parent = NULL;
    struct clb_member_t *entry;
    bool leftmost = true;

    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct clb_member_t, members_in_order_node);
        if (member->num_current_connections < entry->num_current_connections) {
            link = &parent->rb_left;
        } else {
            link = &parent->rb_right;
            leftmost = false;
        }
    }

    rb_link_node(&member->members_in_order_node, parent, link);
    rb_insert_color_cached(&member->members_in_order_node, &vs->members_in_order, leftmost);

    return 0;
}


int clb_virtual_server_unregister_member(struct clb_virtual_server_t *vs, struct clb_member_t *member)
{
    rb_erase_cached(&member->members_in_order_node, &vs->members_in_order);
    hash_del(&member->members_node);
    return 0;
}


struct clb_member_t *clb_virtual_server_find_member_by_address(struct clb_virtual_server_t *vs, struct clb_member_address_t *address)
{
    struct clb_member_t *iter;
    unsigned long address_hash = clb_member_address_hash(address);
    hash_for_each_possible(vs->members, iter, members_node, address_hash) {
        if (clb_member_address_equals(&iter->address, address))
            return iter;
    }
    return NULL;
}


int clb_virtual_server_update_config(struct clb_virtual_server_t *vs, struct clb_virtual_server_config_t *config)
{
    vs->config = *config;
    return 0;
}
