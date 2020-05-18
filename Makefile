obj-m += genl_kernel.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make all_user

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make clean_user

install:
	make all
	make uninstall
	insmod genl_kernel.ko

uninstall:
	rmmod genl_kernel 2> /dev/null || :

run:
	make install
	make run_user
	make uninstall

all_user:
	gcc -g -I/usr/include/libnl3 -l nl-3 -l nl-genl-3 -o genl_user genl_user.c

clean_user:
	rm -f genl_user

run_user:
	make all_user
	./genl_user
