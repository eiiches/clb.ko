#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/cache.h> // __ro_after_init;
#include <net/genetlink.h> // struct genl_family;
#include <net/sock.h> // sock_net;

#include <uapi/clb/netlink.h>
#include "module-netlink.h"
#include "netlink.pb-c.h"


static struct genl_ops *clb_netlink_ops;

static struct genl_family clb_netlink_family __ro_after_init = {
    .hdrsize = 0,
    .name = CLB_NETLINK_NAME,
    .version = CLB_NETLINK_VERSION,
    .maxattr = 0,
    .netnsok = true,
    .parallel_ops = false, // automatically hold genl_lock()
    .module = THIS_MODULE,
    .ops = NULL, // set later in __init
    .n_ops = 0, // set later in __init
};


// static Clb__Status STATUS_OK = CLB__STATUS__INIT;


void clb_service_impl_create_virtual_server(Clb__Clb_Service *service,
                                            const Clb__CreateVirtualServerRequest *input,
                                            Clb__Status_Closure closure,
                                            void *closure_data) {
    pr_info("clb_service_impl_create_virtual_server(config = %px)\n", input->config);
    pr_info("clb_service_impl_create_virtual_server(config->algo = %d)\n", input->config->algorithm);

    Clb__Status status = CLB__STATUS__INIT;
    status.code = 1;
    closure(&status, closure_data);
}

void clb_service_impl_update_virtual_server(Clb__Clb_Service *service,
                                            const Clb__UpdateVirtualServerRequest *input,
                                            Clb__Status_Closure closure,
                                            void *closure_data) {
    pr_info("clb_service_impl_update_virtual_server\n");

    Clb__Status status = CLB__STATUS__INIT;
    status.code = 2;
    closure(&status, closure_data);
}

void clb_service_impl_delete_virtual_server(Clb__Clb_Service *service,
                                            const Clb__DeleteVirtualServerRequest *input,
                                            Clb__Status_Closure closure,
                                            void *closure_data) {
    pr_info("clb_service_impl_delete_virtual_server\n");

    Clb__Status status = CLB__STATUS__INIT;
    status.code = 3;
    closure(&status, closure_data);
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


struct clb_netlink_response_context {
    int command;
    int snd_portid;
    int snd_seq;
    struct net* netns;
};

static void clb_netlink_command_response_handler(const ProtobufCMessage *output, struct clb_netlink_response_context *context) {
    int err;

    struct sk_buff *skb = genlmsg_new(GENLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb) {
        pr_warn("netlink response is lost due to ENOMEM\n");
        return;
    }

    // libnl expects seq of a reply to be same as the corresponding request.
    void *user_hdr = genlmsg_put(skb, 0 /* kernel */, context->snd_seq, &clb_netlink_family, 0 /* flags */, context->command);
    if (!user_hdr) {
        pr_warn("netlink response is lost due to ENOMEM\n");
        nlmsg_free(skb);
        return;
    }

    const int payload_size = protobuf_c_message_get_packed_size(output);
    if (unlikely(skb_tailroom(skb) < payload_size)) {
        pr_warn("netlink response is lost due to too small skb buffer\n");
        nlmsg_free(skb);
        return;
    }

    void *payload_ptr = skb_put(skb, payload_size);
    protobuf_c_message_pack(output, payload_ptr);

    genlmsg_end(skb, user_hdr);

    err = genlmsg_unicast(context->netns, skb, context->snd_portid);
    if (err) {
        pr_warn("netlink response is lost (code = %d)\n", err);
        nlmsg_free(skb);
    }

    // skb is ref-counted, so no need to (actually, must not) free() here
}

static int clb_netlink_command_handler(struct sk_buff *skb, struct genl_info *info) {
    pr_debug("clb_netlink_command_handler: netns = %px\n", genl_info_net(info));

    const int method_index = info->genlhdr->cmd;
    const ProtobufCMessageDescriptor *input_type = clb__clb__descriptor.methods[method_index].input;

    struct clb_netlink_response_context context;
    context.command = method_index;
    context.snd_portid = info->snd_portid;
    context.snd_seq = info->snd_seq;
    context.netns = genl_info_net(info);

    ProtobufCMessage *payload = protobuf_c_message_unpack(input_type, NULL, genlmsg_len(info->genlhdr), genlmsg_data(info->genlhdr));
    clb_service_impl.base.invoke(&clb_service_impl.base, method_index, payload, (ProtobufCClosure) clb_netlink_command_response_handler, &context);
    protobuf_c_message_free_unpacked(payload, NULL);

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
