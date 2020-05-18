// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>
#include <net/netlink.h>
#include <net/genetlink.h>

MODULE_DESCRIPTION("Kernel module to demostrate general purpose netlink usage");
MODULE_AUTHOR("Manjunath Patil<manjunath.b.patil@oracle.com>");
MODULE_LICENSE("GPL");

#define HELLO_MSG_LEN 32
int handle_genl_hello(struct sk_buff *hello_skb, struct genl_info *info);

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

/* attribute policy */
#define GENL_ATTR_HELLO_MSG_LEN HELLO_MSG_LEN
static const struct nla_policy our_genl_policy[OUR_GENL_ATTR_MAX + 1] = {
	[GENL_ATTR_HELLO_MSG] = {
				.type = NLA_STRING,
				.len = GENL_ATTR_HELLO_MSG_LEN,
				},
};

/* genl_ops definition */
static const struct genl_ops our_genl_ops[] = {
	{
		.cmd = GENL_CMD_HELLO,
		.doit = handle_genl_hello,
	},
};

/* family definition */
#define OUR_GENL_FAMILY_NAME "OUR_GENL_FAMILY"
#define OUR_GENL_FAMILY_VERSION 1
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
