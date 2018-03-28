#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/slab.h> // kzalloc

#define CLB_PRIVATE
#include "clb-member.h"


struct clb_member_t *clb_member_new(struct clb_member_address_t *address, struct clb_member_config_t *config)
{
    struct clb_member_t *member = (struct clb_member_t *) kzalloc(sizeof(struct clb_member_t), GFP_KERNEL);
    if (!member)
        goto out;
    member->address = *address;
    member->config = *config;
    INIT_HLIST_NODE(&member->members_node);
out:
    pr_debug("clb_member_new(...) => %px\n", member);
    return member;
}


void clb_member_destroy(struct clb_member_t *member)
{
    pr_debug("clb_member_destroy(%px)\n", member);
    kfree(member);
}


void clb_member_update_config(struct clb_member_t *member, struct clb_member_config_t *config)
{
    member->config = *config;
}
