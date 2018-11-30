obj-m += kscan.o
# KDIR = /usr/src/linux-headers-4.18.0-11-generic

all:
	
		gcc user.c -o user
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
		# $(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
		sudo insmod kscan.ko

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order user
	sudo rmmod kscan
