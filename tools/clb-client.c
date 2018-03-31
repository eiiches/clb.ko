#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/errno.h>
#include <clb/netlink.h>
#include "netlink.pb-c.h"


struct clb_client_t {
    ProtobufCService base;
    struct nl_sock *sock;
    int family;
};


struct context_t {
    ProtobufCClosure closure;
    void *closure_data;
    int method_index;
};


static void response_handler(struct nl_msg *msg, struct context_t *context) {
    struct nlmsghdr *nlhdr = nlmsg_hdr(msg);
    struct genlmsghdr *genlhdr = genlmsg_hdr(nlhdr);

    const ProtobufCMessageDescriptor *output_type = clb__clb__descriptor.methods[context->method_index].output;

    const void *payload_ptr = genlmsg_user_data(genlhdr, 0);
    const int payload_size = genlmsg_user_datalen(genlhdr, 0);

    ProtobufCMessage *response = protobuf_c_message_unpack(output_type, NULL, payload_size, payload_ptr);
    context->closure(response, context->closure_data);
    protobuf_c_message_free_unpacked(response, NULL);
}


static void invoke(ProtobufCService *service,
            unsigned method_index,
            const ProtobufCMessage *input,
            ProtobufCClosure closure,
            void *closure_data) {
    // fprintf(stderr, "sending: %d\n", method_index);
    struct clb_client_t *client = (struct clb_client_t *) service;

    struct nl_msg *msg = nlmsg_alloc();
    void *user_hdr = genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, client->family, 0, NLM_F_REQUEST, method_index, CLB_NETLINK_VERSION);
    if (!user_hdr) {
        fprintf(stderr, "genlmsg_put: Invalid argument\n");
        goto errout_nlmsg_free;
    }

    int payload_size = protobuf_c_message_get_packed_size(input);

    void *payload_ptr = nlmsg_reserve(msg, payload_size, 0);
    protobuf_c_message_pack(input, payload_ptr);

    // nla_put_string(msg, DOC_EXMPL_A_MSG, "hello from userspace");
    int len = nl_send_auto(client->sock, msg);
    if (len < 0) {
        nl_perror(-len, "nl_send_auto");
        goto errout_nlmsg_free;
    }

    // fprintf(stderr, "receiving: %d\n", method_index);
    struct context_t context;
    context.closure = closure;
    context.closure_data = closure_data;
    context.method_index = method_index;
    struct nl_cb *cb = nl_cb_alloc(NL_CB_CUSTOM);
    nl_cb_set(cb, NL_CB_MSG_IN, NL_CB_CUSTOM, (nl_recvmsg_msg_cb_t) response_handler, &context);
    int err = nl_recvmsgs(client->sock, cb);
    if (err) {
        nl_perror(err, "nl_recvmsgs");
        goto errout_cb_free;
    }

errout_cb_free:
    nl_cb_put(cb); // free()


errout_nlmsg_free:
    nlmsg_free(msg);
}


struct clb_client_t *clb_client_new(void) {
    ProtobufCService base = {
        &clb__clb__descriptor,
        invoke,
        NULL,
    };
    struct clb_client_t *client = (struct clb_client_t *) malloc(sizeof(struct clb_client_t));
    client->base = base;
    client->sock = NULL;
    client->family = 0;
    return client;
}


int clb_client_connect(struct clb_client_t *client) {
    client->sock = nl_socket_alloc();
    int err = genl_connect(client->sock);
    if (err)
        return err;
    client->family = genl_ctrl_resolve(client->sock, "CLB");
    if (client->family < 0)
        return -client->family;
    nl_socket_disable_auto_ack(client->sock);
    return 0;
}


void clb_client_destroy(struct clb_client_t *client) {
    nl_socket_free(client->sock);
    free(client);
}


static void status_handler(const Clb__Status *status, void *code) {
    *((int *) code) = status->code;
}

int clb_client_create_virtual_server(struct clb_client_t *client) {
    Clb__VirtualServerConfig config = CLB__VIRTUAL_SERVER_CONFIG__INIT;
    config.algorithm = CLB__BALANCING_ALGORITHM__LEAST_CONN;
    Clb__CreateVirtualServerRequest request = CLB__CREATE_VIRTUAL_SERVER_REQUEST__INIT;
    request.config = &config;
    int code = -1;
    clb__clb__create_virtual_server((ProtobufCService *) client, &request, status_handler, &code);
    return code;
}


int clb_client_update_virtual_server(struct clb_client_t *client) {
    Clb__UpdateVirtualServerRequest request = CLB__UPDATE_VIRTUAL_SERVER_REQUEST__INIT;
    int code = -1;
    clb__clb__update_virtual_server((ProtobufCService *) client, &request, status_handler, &code);
    return code;
}


int clb_client_delete_virtual_server(struct clb_client_t *client) {
    Clb__DeleteVirtualServerRequest request = CLB__DELETE_VIRTUAL_SERVER_REQUEST__INIT;
    int code = -1;
    clb__clb__delete_virtual_server((ProtobufCService *) client, &request, status_handler, &code);
    return code;
}
