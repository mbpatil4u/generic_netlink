// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include <net/netlink.h>
#include <net/genetlink.h>

MODULE_DESCRIPTION("Kernel module to demostrate general purpose netlink usage");
MODULE_AUTHOR("Manjunath Patil<manjunath.b.patil@oracle.com>");
MODULE_LICENSE("GPL");

#define HELLO_MSG_LEN 32
#define PINGPONG_MSG_LEN HELLO_MSG_LEN

#define OUR_GENL_FAMILY_NAME "OUR_GENL_FAMILY"
#define OUR_GENL_FAMILY_VERSION 1

int handle_genl_pingpong(struct sk_buff *ping_skb, struct genl_info *info);
int handle_genl_hello(struct sk_buff *hello_skb, struct genl_info *info);
int handle_genl_pingpong_random(struct sk_buff *ping_skb,
				struct genl_info *info);

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
				.len = HELLO_MSG_LEN,
				},
	[GENL_ATTR_PINGPONG_MSG] = {
				.type = NLA_STRING,
				.len = PINGPONG_MSG_LEN,
				},
	[GENL_ATTR_PINGPONG_RANDOM_MSG] = {
				.type = NLA_STRING,
				.len = PINGPONG_MSG_LEN,
				},
};

/* genl_ops definition */
static const struct genl_ops our_genl_ops[] = {
	{
		.cmd = GENL_CMD_HELLO,
		.doit = handle_genl_hello,
	},
	{
		.cmd = GENL_CMD_PINGPONG,
		.doit = handle_genl_pingpong,
	},
	{
		.cmd = GENL_CMD_PINGPONG_RANDOM,
		.doit = handle_genl_pingpong_random,
	},
};

/* family definition */
static struct genl_family our_genl_family = {
	.name = OUR_GENL_FAMILY_NAME,
	.version = OUR_GENL_FAMILY_VERSION,
	.maxattr = OUR_GENL_ATTR_MAX,
	.netnsok = true,
	.module = THIS_MODULE,

	.policy = our_genl_policy,
	.ops = our_genl_ops,
	.n_ops = ARRAY_SIZE(our_genl_ops),
};

int handle_genl_pingpong_random(struct sk_buff *ping_skb, struct genl_info *info)
{
	char *ping_msg, *pong_msg = "Pong from kernel!";
	struct nlattr *na;
	struct sk_buff *pong_skb;
	void *msghead;
	int n;

	na = info->attrs[GENL_ATTR_PINGPONG_RANDOM_MSG];
	if (!na) {
		pr_err("genl_kernel: %s Empty msg from %d\n", __func__,
		       info->snd_portid);
		return -EINVAL;
	}

	/* received ping msg */
	ping_msg = (char *)nla_data(na);
	pr_info("genl_kenel:%s src=%u msg=%s\n", __func__, info->snd_portid,
		ping_msg);

	/* send few random pongs to demonstrate multi-part msgs */
	n = jiffies % 10; /* up to 10 pongs */
	do {
		pong_skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
		msghead = genlmsg_put(pong_skb, 0, info->snd_seq,
					&our_genl_family, NLM_F_MULTI,
					GENL_CMD_PINGPONG_RANDOM);
		nla_put_string(pong_skb, GENL_ATTR_PINGPONG_RANDOM_MSG,
				pong_msg);
		genlmsg_end(pong_skb, msghead);

		pr_info("genl_kenel:%s sending pong_msg=%s\n", __func__, pong_msg);
		genlmsg_unicast(genl_info_net(info), pong_skb, info->snd_portid);
	} while (n--);

	/* finish multi-part msg */
	pong_skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	nlmsg_put(pong_skb, info->snd_portid, info->snd_seq, NLMSG_DONE, 0, 0);
	genlmsg_unicast(genl_info_net(info), pong_skb, info->snd_portid);

	return 0;
}
int handle_genl_pingpong(struct sk_buff *ping_skb, struct genl_info *info)
{
	char *ping_msg, *pong_msg = "Pong from kernel!";
	struct nlattr *na;
	struct sk_buff *pong_skb;
	void *msghead;

	na = info->attrs[GENL_ATTR_PINGPONG_MSG];
	if (!na) {
		pr_err("genl_kernel: %s Empty msg from %d\n", __func__,
		       info->snd_portid);
		return -EINVAL;
	}

	/* received ping msg */
	ping_msg = (char *)nla_data(na);
	pr_info("genl_kenel:%s src=%u msg=%s\n", __func__, info->snd_portid,
		ping_msg);

	/* alloc msg buffer
	 * fill nl and genl header
	 * fill our data
	 * finalize the msg buffer
	 * send pong
	 */
	pong_skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	msghead = genlmsg_put(pong_skb, 0, info->snd_seq, &our_genl_family,
				0, GENL_CMD_PINGPONG);
	nla_put_string(pong_skb, GENL_ATTR_PINGPONG_MSG, pong_msg);
	genlmsg_end(pong_skb, msghead);
	pr_info("genl_kenel:%s sending pong_msg=%s\n", __func__, pong_msg);
	genlmsg_unicast(genl_info_net(info), pong_skb, info->snd_portid);

	return 0;
}

int handle_genl_hello(struct sk_buff *hello_skb, struct genl_info *info)
{
	char *hello_msg;
	struct nlattr *na;

	na = info->attrs[GENL_ATTR_HELLO_MSG];
	if (!na) {
		pr_err("genl_kernel: %s Empty msg from %d\n", __func__,
		       info->snd_portid);
		return -EINVAL;
	}

	/* received hello msg */
	hello_msg = (char *)nla_data(na);
	pr_info("genl_kenel:%s src=%u msg=%s\n", __func__, info->snd_portid,
		hello_msg);
	return 0;
}

static int genl_init(void)
{
	pr_info("genl_kernel:%s Loaded genl test kernel module\n", __func__);
	genl_register_family(&our_genl_family);
	return 0;
}

static void genl_exit(void)
{
	genl_unregister_family(&our_genl_family);
	pr_info("genl_kernel:%s Unloaded genl test kernel module\n", __func__);
}

module_init(genl_init);
module_exit(genl_exit);
