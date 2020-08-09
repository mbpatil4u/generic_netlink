
Generic Netlink Usage: kernel-userspace interaction

# git log --oneline
32f6d6f genl: Demo for structure exchange
7cbeec0 genl: Demo for multi-part nl msgs
53762d2 genl: pingpong implementation
c2af176 genl: Hello packet
f43d76d genl: kernel module and userspace program

# make help
possible targets
all 		- build genl_kernel.ko and genl_user
clean 		- clean genl_kernel.ko and genl_user
install		- 'make all; make uninstall' and then 'insmod genl_kernel.ko'
uninstall 	- rmmod genl_kernel.ko
run 		- 'make install;make run_user;make uninstall'
all_user 	- build genl_user
clean_user 	- clean genl_user
run_user 	- make all_user; ./genl_user -s

Use-cases:
1. Hello packet
    Output:
    [root@ca-dev87 gen_netlink]# make run
    ...
    ./genl_user
    general netlink userspace program
    genl_user:send_hello msg=Hello World from userspace!
    ...
    [root@ca-dev87 gen_netlink]# dmesg | tail -3
    [958180.771086] genl_kernel:genl_init Loaded genl test kernel module
    [958180.910003] genl_kenel:handle_genl_hello src=2105563912 msg=Hello
    World from userspace!
    [958181.014823] genl_kernel:genl_exit Unloaded genl test kernel module

2. ping-pong
    output:
    [root@ca-dev87 gen_netlink]# make run
    ..
    ./genl_user -p
    general netlink userspace program
    genl_user:pingpong msg=Ping from userspace!
    genl_user:handle_pong pong msg=Pong from kernel!
    ..
    
    From console:
    [974090.478953] genl_kernel:genl_init Loaded genl test kernel module
    [974090.610927] genl_kenel:handle_genl_pingpong src=4282406798 msg=Ping
    from userspace!
    [974090.705484] genl_kenel:handle_genl_pingpong sending pong_msg=Pong
    from kernel!
    [974090.798020] genl_kernel:genl_exit Unloaded genl test kernel module


3. multi-part data
    Output:
    [root@ca-dev87 gen_netlink]# make run
    ..
    ./genl_user -r
    general netlink userspace program
    genl_user:pingpong_random msg=Ping from userspace!
    genl_user:handle_pong_random pong msg=Pong from kernel!
    genl_user:handle_pong_random pong msg=Pong from kernel!
    genl_user:handle_pong_random pong msg=Pong from kernel!
    genl_user:handle_pong_random pong msg=Pong from kernel!
    ..

4. Binary data [struct]
    Output:
    [root@ca-dev87 gen_netlink]# make run
    ..
    ./genl_user -s
    general netlink userspace program
    genl_user:struct_msg 'struct_msg from userspace!' sending REQ msg
    genl_user:handle_struct_msg received REPLY struct_msg= [123 8114927186
    my_prog_name 123.45.67.89 abcd:ef01:2345:6789:123:4567:89ab:cdef]
    
    console:
    [3820280.134694] genl_kernel:genl_init Loaded genl test kernel module
    [3820280.268439] genl_kenel:handle_genl_struct src=3078649462
    msg='struct_msg from userspace!' received REQ msg
    [3820280.387228] genl_kenel:handle_genl_struct REPLY struct_msg =
    [1238114927186 my_prog_name
    123.45.67.89abcd:ef01:2345:6789:0123:4567:89ab:cdef] [3820280.546112]
    genl_kenel:handle_genl_struct sending REPLY struct_msg
    [3820280.628217] genl_kernel:genl_exit Unloaded genl test kernel module

