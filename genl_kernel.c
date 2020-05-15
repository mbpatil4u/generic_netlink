// SPDX-License-Identifier: GPL-2.0
#include<linux/module.h>

MODULE_DESCRIPTION("Kernel module to demostrate general purpose netlink usage");
MODULE_AUTHOR("Manjunath Patil<manjunath.b.patil@oracle.com>");
MODULE_LICENSE("GPL");

static int genl_init(void)
{
	pr_info("genl_kernel:%s Loaded genl test kernel module\n", __func__);
	return 0;
}

static void genl_exit(void)
{
	pr_info("genl_kernel:%s Unloaded genl test kernel module\n", __func__);
}

module_init(genl_init);
module_exit(genl_exit);
