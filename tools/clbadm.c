#include <netlink/genl/genl.h>
#include "clb-client.h"


int
main(int argc, char **argv)
{
    int ret = 0;
    struct clb_client_t *client = clb_client_new();

    ret = clb_client_connect(client);
    if (ret) {
        nl_perror(ret, "clb_client_connect");
        goto finally;
    }

    clb_client_create_virtual_server(client);
    clb_client_update_virtual_server(client);
    clb_client_delete_virtual_server(client);

finally:
    clb_client_destroy(client);
    return ret;
}
