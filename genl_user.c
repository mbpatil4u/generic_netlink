// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <errno.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <arpa/inet.h>

#define HELLO_MSG_LEN 32
#define PINGPONG_MSG_LEN HELLO_MSG_LEN
#define OUR_GENL_FAMILY_NAME "OUR_GENL_FAMILY"

#define MAX_LEN 32
struct genl_cmd_struct {
	__u32			pid;
	__u64			ts;
	char			name[MAX_LEN];
	struct	in_addr		ipv4;		/* 4 bytes */
	struct	in6_addr	ipv6;		/* 16 bytes */
};

/* genl cmds */
enum our_genl_cmds {
	GENL_CMD_UNSPEC,	/* avoid using 0 */
	GENL_CMD_HELLO,
	GENL_CMD_PINGPONG,
	GENL_CMD_PINGPONG_RANDOM,
	GENL_CMD_STRUCT,	/* receiving structured data */
};

/* attributes */
enum our_genl_attrs {
	GENL_ATTR_UNSPEC,	/* avoid using 0 */
	GENL_ATTR_HELLO_MSG,
	GENL_ATTR_PINGPONG_MSG,
	GENL_ATTR_PINGPONG_RANDOM_MSG,
	GENL_ATTR_STRUCT_MSG,	/* nest */

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
	[GENL_ATTR_STRUCT_MSG] = {
				.type = NLA_BINARY,
				.maxlen = sizeof(struct genl_cmd_struct)
				}
};

void ipv4_ipv6_print_test(void)
{
	char ipv4[INET_ADDRSTRLEN], ipv6[INET6_ADDRSTRLEN];
	struct genl_cmd_struct data;

	if (inet_pton(AF_INET, "123.45.67.89", &data.ipv4)) {
		inet_ntop(AF_INET, &data.ipv4, ipv4, INET_ADDRSTRLEN);
		printf("ipv4: %s\n", ipv4);
	}

	if (inet_pton(AF_INET6, "abcd:ef01:2345:6789:0123:4567:89ab:cdef",
		      &data.ipv6)) {
		inet_ntop(AF_INET6, &data.ipv6, ipv6, INET6_ADDRSTRLEN);
		printf("ipv6: %s\n", ipv6);
	}
}

int handle_struct_msg(struct nl_msg *nlmsg, void *arg)
{
	struct nlattr *attr[OUR_GENL_ATTR_MAX+1];
	struct nlattr *na;
	struct genl_cmd_struct *data;
	char ipv4[INET_ADDRSTRLEN], ipv6[INET6_ADDRSTRLEN];

	genlmsg_parse(nlmsg_hdr(nlmsg), 0, attr,
			OUR_GENL_ATTR_MAX, our_genl_policy);
	na = attr[GENL_ATTR_STRUCT_MSG];
	if (!na) {
		printf("genl_user:%s Empty msg from kernel\n", __func__);
		return -EINVAL;
	}
	data = nla_data(na);

	inet_ntop(AF_INET, &data->ipv4, ipv4, INET_ADDRSTRLEN);
	inet_ntop(AF_INET6, &data->ipv6, ipv6, INET6_ADDRSTRLEN);

	printf("genl_user:%s received REPLY struct_msg= [%u %llu %s %s %s]\n",
		__func__, data->pid, data->ts, data->name, ipv4, ipv6);

	return NL_OK;
}

void struct_msg(struct nl_sock *nlsock, int family_id)
{
	struct nl_msg *nlmsg;
	struct nl_cb *cb;
	char *out_msg = "msg from userspace!";

	/* prepare and send ping */
	nlmsg = nlmsg_alloc();
	genlmsg_put(nlmsg, NL_AUTO_PID, NL_AUTO_SEQ, family_id, 0,
		    NLM_F_REQUEST, GENL_CMD_STRUCT, 0);
	nla_put_string(nlmsg, GENL_ATTR_STRUCT_MSG, out_msg);
	printf("genl_user:%s '%s' sending REQ msg\n", __func__, out_msg);
	nl_send_auto(nlsock, nlmsg);

	/* receive msg from kernel */
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_socket_disable_seq_check(nlsock);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handle_struct_msg, NULL);
	nl_recvmsgs(nlsock, cb);
	nl_cb_put(cb);
	nlmsg_free(nlmsg);
}

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
	printf("\t-s -- receive structure data from kernel\n");
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

	while ((c = getopt(argc, argv, "hprs")) != -1) {
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

		case 's':
			struct_msg(nlsock, family_id);
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
