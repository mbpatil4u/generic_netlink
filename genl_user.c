// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>

#define HELLO_MSG_LEN 32
#define PINGPONG_MSG_LEN HELLO_MSG_LEN
#define OUR_GENL_FAMILY_NAME "OUR_GENL_FAMILY"

/* genl cmds */
enum our_genl_cmds {
	GENL_CMD_UNSPEC,	/* avoid using 0 */
	GENL_CMD_HELLO,
	GENL_CMD_PINGPONG,
	GENL_CMD_PINGPONG_RANDOM,
};

/* attributes */
enum our_genl_attrs {
	GENL_ATTR_UNSPEC,	/* avoid using 0 */
	GENL_ATTR_HELLO_MSG,
	GENL_ATTR_PINGPONG_MSG,
	GENL_ATTR_PINGPONG_RANDOM_MSG,
	__GENL_ATTR__MAX,
};
#define OUR_GENL_ATTR_MAX (__GENL_ATTR__MAX - 1)

/* attribute policy */
static const struct nla_policy our_genl_policy[OUR_GENL_ATTR_MAX + 1] = {
	[GENL_ATTR_HELLO_MSG] = {
				.type = NLA_STRING,
				.maxlen = HELLO_MSG_LEN,
				},
	[GENL_ATTR_PINGPONG_MSG] = {
				.type = NLA_STRING,
				.maxlen = PINGPONG_MSG_LEN,
				},
	[GENL_ATTR_PINGPONG_RANDOM_MSG] = {
				.type = NLA_STRING,
				.maxlen = PINGPONG_MSG_LEN,
				},
};

int handle_pong_random(struct nl_msg *nlmsg, void *arg)
{
	struct nlattr *attr[OUR_GENL_ATTR_MAX+1];
	struct nlattr *na;
	char *pong_msg;

	genlmsg_parse(nlmsg_hdr(nlmsg), 0, attr,
			OUR_GENL_ATTR_MAX, our_genl_policy);

	na = attr[GENL_ATTR_PINGPONG_RANDOM_MSG];
	pong_msg = nla_get_string(na);
	printf("genl_user:%s pong msg=%s\n", __func__, pong_msg);

	return NL_OK;
}

void pingpong_random(struct nl_sock *nlsock, int family_id)
{
	struct nl_msg *nlmsg;
	struct nl_cb *cb;
	char *ping_msg = "Ping from userspace!";

	/* prepare and send ping */
	nlmsg = nlmsg_alloc();
	genlmsg_put(nlmsg, NL_AUTO_PID, NL_AUTO_SEQ, family_id, 0,
		    NLM_F_REQUEST, GENL_CMD_PINGPONG_RANDOM, 0);
	nla_put_string(nlmsg, GENL_ATTR_PINGPONG_RANDOM_MSG, ping_msg);
	printf("genl_user:%s msg=%s\n", __func__, ping_msg);
	nl_send_auto(nlsock, nlmsg);

	/* receive pong from kernel */
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_socket_disable_seq_check(nlsock);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handle_pong_random, NULL);
	nl_recvmsgs(nlsock, cb);
	nl_cb_put(cb);
	nlmsg_free(nlmsg);
}

int handle_pong(struct nl_msg *nlmsg, void *arg)
{
	struct nlattr *attr[OUR_GENL_ATTR_MAX+1];
	struct nlattr *na;
	char *pong_msg;

	genlmsg_parse(nlmsg_hdr(nlmsg), 0, attr,
			OUR_GENL_ATTR_MAX, our_genl_policy);

	na = attr[GENL_ATTR_PINGPONG_MSG];
	pong_msg = nla_get_string(na);
	printf("genl_user:%s pong msg=%s\n", __func__, pong_msg);

	return NL_OK;
}

void pingpong(struct nl_sock *nlsock, int family_id)
{
	struct nl_msg *nlmsg;
	struct nl_cb *cb;
	char *ping_msg = "Ping from userspace!";

	/* prepare and send ping */
	nlmsg = nlmsg_alloc();
	genlmsg_put(nlmsg, NL_AUTO_PID, NL_AUTO_SEQ, family_id, 0,
		    NLM_F_REQUEST, GENL_CMD_PINGPONG, 0);
	nla_put_string(nlmsg, GENL_ATTR_PINGPONG_MSG, ping_msg);
	printf("genl_user:%s msg=%s\n", __func__, ping_msg);
	nl_send_auto(nlsock, nlmsg);

	/* receive pong from kernel */
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_socket_disable_seq_check(nlsock);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handle_pong, NULL);
	nl_recvmsgs(nlsock, cb);
	nl_cb_put(cb);
	nlmsg_free(nlmsg);
}

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

void print_usage(char *prog_name)
{
	printf("Usage:%s option\noptions\n", prog_name);
	printf("\t-h -- hello packet from userspace to kernel\n");
	printf("\t-p -- userspace sends ping. kernel responds with pong\n");
	printf("\t-r -- userspace sends ping. kernel responds with more than "
			"one pongs[randomly up to 10 pongs]\n");
}

int main(int argc,  char **argv)
{
	struct nl_sock *nlsock;
	int family_id;
	int c;

	if (argc < 2) {
		print_usage(argv[0]);
		return -EINVAL;
	}

	printf("general netlink userspace program\n");

	/* prepare nlsock and connect it with genl */
	nlsock = nl_socket_alloc();
	genl_connect(nlsock);
	family_id = genl_ctrl_resolve(nlsock, OUR_GENL_FAMILY_NAME);

	while ((c = getopt(argc, argv, "hpr")) != -1) {
		switch (c) {
		case 'h':
			send_hello(nlsock, family_id);
			break;

		case 'p':
			pingpong(nlsock, family_id);
			break;

		case 'r':
			pingpong_random(nlsock, family_id);
			break;

		default:
			print_usage(argv[0]);
			break;
		}
		break; /* process only one argument */
	}

	nl_socket_free(nlsock);
	return 0;
}
