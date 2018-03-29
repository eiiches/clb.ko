#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/errno.h>
#include <clb/netlink.h>
#include "netlink.pb-c.h"


void invoke(ProtobufCService *service,
            unsigned method_index,
            const ProtobufCMessage *input,
            ProtobufCClosure closure,
            void *closure_data) {
    fprintf(stderr, "invoked: %d\n", method_index);
}


static ProtobufCService client = {
    &clb__clb__descriptor,
    invoke,
    NULL,
};


int
main(int argc, char **argv)
{
    struct nl_sock *sock = nl_socket_alloc();
    int err = genl_connect(sock);
    if (err) {
        nl_perror(err, "genl_connect");
        exit(-1);
    }

    int family = genl_ctrl_resolve(sock, "CLB");
    if (family < 0) {
        nl_perror(-family, "genl_ctrl_resolve");
        exit(-1);
    }
    printf("family = %d\n", family);

    // struct genlmsghdr hdr = {
    //     .cmd = 0, // CLB_NETLINK_COMMAND_CREATE_VS
    //     .version = 0x01,
    // };

    Clb__CreateVirtualServerRequest request;
    clb__clb__create_virtual_server(&client, &request, NULL, NULL);

// #define CLB_NETLINK_VERSION 0x01
// #define CLB_NETLINK_COMMAND_CREATE_VS 0
    struct nl_msg *msg = nlmsg_alloc();
    void *user_hdr = genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, NLM_F_REQUEST, CLB_NETLINK_COMMAND_CREATE_VS, CLB_NETLINK_VERSION);
    if (!user_hdr) {
        fprintf(stderr, "genlmsg_put: Invalid argument\n");
        err = -1;
        goto errout_nlmsg_free;
    }
    // nla_put_string(msg, DOC_EXMPL_A_MSG, "hello from userspace");
    int len = nl_send_auto(sock, msg);
    if (len < 0) {
        nl_perror(-len, "nl_send_auto");
        goto errout_nlmsg_free;
    }

errout_nlmsg_free:
    nlmsg_free(msg);

    // int flags = 0;
    // err = nl_send_simple(sock, family, flags, &hdr, sizeof(hdr));
    // if (err)
    //     exit(err);

    nl_socket_free(sock);
    return err;
}
