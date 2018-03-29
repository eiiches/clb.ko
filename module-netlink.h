#ifndef _MODULE_NETLINK_H_
#define _MODULE_NETLINK_H_


#include <linux/init.h>


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


extern int __init clb_module_netlink_init(void);

extern void clb_module_netlink_exit(void);


#endif /* _MODULE_NETLINK_H_ */
