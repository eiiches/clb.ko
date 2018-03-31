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


static void invoke(ProtobufCService *service,
            unsigned method_index,
            const ProtobufCMessage *input,
            ProtobufCClosure closure,
            void *closure_data) {
    fprintf(stderr, "sending: %d\n", method_index);
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

    fprintf(stderr, "receiving: %d\n", method_index);
    nl_recvmsgs_default(client->sock);


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


void clb_client_create_virtual_server(struct clb_client_t *client) {
    Clb__VirtualServerConfig config = CLB__VIRTUAL_SERVER_CONFIG__INIT;
    config.algorithm = CLB__BALANCING_ALGORITHM__LEAST_CONN;
    Clb__CreateVirtualServerRequest request = CLB__CREATE_VIRTUAL_SERVER_REQUEST__INIT;
    request.config = &config;
    clb__clb__create_virtual_server((ProtobufCService *) client, &request, NULL, NULL);
}


void clb_client_update_virtual_server(struct clb_client_t *client) {
    Clb__UpdateVirtualServerRequest request = CLB__UPDATE_VIRTUAL_SERVER_REQUEST__INIT;
    clb__clb__update_virtual_server((ProtobufCService *) client, &request, NULL, NULL);
}


void clb_client_delete_virtual_server(struct clb_client_t *client) {
    Clb__DeleteVirtualServerRequest request = CLB__DELETE_VIRTUAL_SERVER_REQUEST__INIT;
    clb__clb__delete_virtual_server((ProtobufCService *) client, &request, NULL, NULL);
}
