obj-m := netlink.o
KERNELBUILD := /lib/modules/`uname -r`/build
PWD	:= `pwd`
default:
	make -C $(KERNELBUILD) SUBDIRS=$(PWD) modules
	make clean
clean:
	rm -rf *.o .depend .*.cmd *.mod.c .tmp* *.symvers *.order 
