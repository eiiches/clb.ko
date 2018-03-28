#ifndef _CLB_VIRTUAL_SERVER_CONFIG_H_
#define _CLB_VIRTUAL_SERVER_CONFIG_H_


struct clb_virtual_server_config_t {
    int method; // CLB_POLICY_ROUND_ROBIN or CLB_POLICY_LEAST_CONN
};


#endif /* _CLB_VIRTUAL_SERVER_CONFIG_H_ */
