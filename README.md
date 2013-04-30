通过netlink实现用户进程与内核进程通信

user.c为用户进程
netlink.c为内核模块
执行make之后即可生成netlink.ko和user二进制文件
加载模块 sudo insmod netlinl.ko
运行 ./user 即可看到输出信息
卸载模块 sudo rmmod netlink
