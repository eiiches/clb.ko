#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/cache.h> // __ro_after_init;
#include <net/genetlink.h> // struct genl_family;
#include <net/sock.h> // sock_net;

#include <uapi/clb/netlink.h>
#include "module-netlink.h"

#include "netlink.pb-c.h"


// static int clb_netlink_cmd_create_vs(struct sk_buff *skb, struct genl_info *info) {
//     pr_info("clb_netlink_cmd_create_vs(net = %px)\n", sock_net(skb->sk));
//     return 0;
// }


// static const struct nla_policy clb_netlink_cmd_policy[NUM_CLB_NETLINK_COMMAND_ATTRS] = {
//     [CLB_NETLINK_COMMAND_ATTR_VS] = { .type = NLA_NESTED },
// };


static struct genl_ops *clb_netlink_ops;

//     {
//         .cmd = CLB_NETLINK_COMMAND_CREATE_VS,
//         .flags = GENL_ADMIN_PERM,
//         .doit = clb_netlink_cmd_create_vs,
//     }
// };


static struct genl_family clb_netlink_family __ro_after_init = {
    .hdrsize = 0,
    .name = CLB_NETLINK_NAME,
    .version = CLB_NETLINK_VERSION,
    .maxattr = 0,
    .netnsok = true,
    .module = THIS_MODULE,
    .ops = NULL, // set by __init
    .n_ops = 0, // set by __init
};


void clb_service_impl_create_virtual_server(Clb__Clb_Service *service,
                                            const Clb__CreateVirtualServerRequest *input,
                                            Clb__Status_Closure closure,
                                            void *closure_data) {
    pr_info("clb_service_impl_create_virtual_server\n");
}

void clb_service_impl_update_virtual_server(Clb__Clb_Service *service,
                                            const Clb__UpdateVirtualServerRequest *input,
                                            Clb__Status_Closure closure,
                                            void *closure_data) {
    pr_info("clb_service_impl_update_virtual_server\n");
}

void clb_service_impl_delete_virtual_server(Clb__Clb_Service *service,
                                            const Clb__DeleteVirtualServerRequest *input,
                                            Clb__Status_Closure closure,
                                            void *closure_data) {
    pr_info("clb_service_impl_delete_virtual_server\n");
}

void clb_service_impl_add_member(Clb__Clb_Service *service,
                                 const Clb__AddMemberRequest *input,
                                 Clb__Status_Closure closure,
                                 void *closure_data) {
    pr_info("clb_service_impl_add_member\n");
}

void clb_service_impl_change_member(Clb__Clb_Service *service,
                                    const Clb__ChangeMemberRequest *input,
                                    Clb__Status_Closure closure,
                                    void *closure_data) {
    pr_info("clb_service_impl_change_member\n");
}

void clb_service_impl_remove_member(Clb__Clb_Service *service,
                                    const Clb__RemoveMemberRequest *input,
                                    Clb__Status_Closure closure,
                                    void *closure_data) {
    pr_info("clb_service_impl_remove_member\n");
}


static Clb__Clb_Service clb_service_impl = CLB__CLB__INIT(clb_service_impl_);


static int clb_netlink_command_handler(struct sk_buff *skb, struct genl_info *info) {
    pr_info("clb_netlink_command_handler: netns = %px\n", sock_net(skb->sk));
    int command = info->genlhdr->cmd;
    clb_service_impl.base.invoke(&clb_service_impl.base, command, NULL, NULL, NULL);
    // clb_service_impl.base.invoke(&clb_service_impl.base, command, 
	// 			   const ProtobufCMessage *input,
	// 			   ProtobufCClosure closure,
	// 			   void *closure_data);
    return 0;
}


int __init clb_module_netlink_init(void) {
    clb_netlink_ops = (struct genl_ops *) kzalloc(sizeof(struct genl_ops) * clb__clb__descriptor.n_methods, GFP_KERNEL);
    for (int i = 0; i < clb__clb__descriptor.n_methods; ++i) {
        clb_netlink_ops[i].cmd = i;
        clb_netlink_ops[i].flags = GENL_ADMIN_PERM;
        clb_netlink_ops[i].doit = clb_netlink_command_handler;
    }
    clb_netlink_family.ops = clb_netlink_ops;
    clb_netlink_family.n_ops = clb__clb__descriptor.n_methods;
    return genl_register_family(&clb_netlink_family);
}


void clb_module_netlink_exit(void) {
    genl_unregister_family(&clb_netlink_family);
    kfree(clb_netlink_ops);
}
