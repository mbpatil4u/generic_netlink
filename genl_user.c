// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>

#define HELLO_MSG_LEN 32
#define OUR_GENL_FAMILY_NAME "OUR_GENL_FAMILY"

/* genl cmds */
enum our_genl_cmds {
	GENL_CMD_UNSPEC,	/* avoid using 0 */
	GENL_CMD_HELLO,
};

/* attributes */
enum our_genl_attrs {
	GENL_ATTR_UNSPEC,	/* avoid using 0 */
	GENL_ATTR_HELLO_MSG,
	__GENL_ATTR__MAX,
};
#define OUR_GENL_ATTR_MAX (__GENL_ATTR__MAX - 1)

void send_hello(struct nl_sock *nlsock, int family_id)
{
	struct nl_msg *nlmsg;
	char *hello_msg = "Hello World from userspace!";
	int ret;

	/* prepare and send hello */
	nlmsg = nlmsg_alloc();
	genlmsg_put(nlmsg, NL_AUTO_PID, NL_AUTO_SEQ, family_id, 0,
		    NLM_F_REQUEST, GENL_CMD_HELLO, 0);
	nla_put_string(nlmsg, GENL_ATTR_HELLO_MSG, hello_msg);
	printf("genl_user:%s msg=%s\n", __func__, hello_msg);
	nl_send_auto(nlsock, nlmsg);
	nlmsg_free(nlmsg);
}

int main(int argc,  char **argv)
{
	struct nl_sock *nlsock;
	int family_id;

	printf("general netlink userspace program\n");

	/* prepare nlsock and connect it with genl */
	nlsock = nl_socket_alloc();
	genl_connect(nlsock);
	family_id = genl_ctrl_resolve(nlsock, OUR_GENL_FAMILY_NAME);

	send_hello(nlsock, family_id);

	nl_socket_free(nlsock);
	return 0;
}
