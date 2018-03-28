#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#define CLB_PRIVATE
#include "clb-api.h"


int clb_create_virtual_server(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address,
                              struct clb_virtual_server_config_t *config)
{
    // TODO: validate address
    struct clb_virtual_server_t *vs = clb_virtual_server_new(address, config);
    if (!vs)
        return -ENOMEM;
    int err = clb_register_virtual_server(clb, vs);
    if (err)
        goto fail;
    return 0;
fail:
    clb_virtual_server_destroy(vs);
    return err;
}


int clb_update_virtual_server(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address,
                              struct clb_virtual_server_config_t *config)
{
    // TODO: validate address
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, address);
    if (!vs)
        return -ENOENT;
    return clb_virtual_server_update_config(vs, config);
}


int clb_delete_virtual_server(struct clb_t *clb,
                              struct clb_virtual_server_address_t *address)
{
    // TODO: validate address
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, address);
    if (!vs)
        return -ENOENT;
    clb_unregister_virtual_server(clb, vs);
    clb_virtual_server_destroy(vs);
    return 0;
}


int clb_virtual_server_add_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *address, struct clb_member_config_t *config)
{
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, server);
    if (!vs)
        return -ENOENT;
    struct clb_member_t *member = clb_member_new(address, config);
    if (!member)
        return -ENOMEM;
    return clb_virtual_server_register_member(vs, member);
}


int clb_virtual_server_change_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *address, struct clb_member_config_t *config)
{
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, server);
    if (!vs)
        return -ENOENT;
    struct clb_member_t *member = clb_virtual_server_find_member_by_address(vs, address);
    if (!member)
        return -ENOENT;
    clb_member_update_config(member, config);
    return 0;
}


int clb_virtual_server_remove_member(struct clb_t *clb, struct clb_virtual_server_address_t *server, struct clb_member_address_t *address)
{
    struct clb_virtual_server_t *vs = clb_find_virtual_server_by_address(clb, server);
    if (!vs)
        return -ENOENT;
    struct clb_member_t *member = clb_virtual_server_find_member_by_address(vs, address);
    if (!member)
        return -ENOENT;
    clb_virtual_server_unregister_member(vs, member);
    clb_member_destroy(member);
    return 0;
}
