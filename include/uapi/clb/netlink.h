#ifndef _UAPI_CLB_NETLINK_H_
#define _UAPI_CLB_NETLINK_H_


#define CLB_NETLINK_NAME "CLB"

#define CLB_NETLINK_VERSION 0x01


enum clb_netlink_cmds {
    CLB_NETLINK_COMMAND_CREATE_VS,
    NUM_CLB_NETLINK_COMMANDS,
};


enum clb_netlink_cmd_attr {
    CLB_NETLINK_CMD_ATTR_VS = 0,
    NUM_CLB_NETLINK_CMD_ATTRS
};


#endif /* _UAPI_CLB_NETLINK_H_ */
