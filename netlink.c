#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/types.h>
#include<linux/sched.h>
#include<linux/netlink.h>
#include<net/sock.h>

#define NETLINK_TEST 17

MODULE_LICENSE("GPL");
MODULE_AUTHOR("github/gwcai");

struct {
	__u32 pid;
}user_process;
static struct sock *nl_fd = NULL;

static int send_to_user(char *buf)
{
	int size;
	struct sk_buff *skb;
	unsigned char *old_tail;
	struct nlmsghdr *nlh ;   //报文头
	int ret;
	size = NLMSG_SPACE(strlen(buf)); //报文大小
	/*分配新的套接字缓存使用GFP_ATOMIC标志进程不>会被置为睡眠*/
	skb = alloc_skb(size,GFP_ATOMIC);
	
	/*初始化一个netlink消息首部*/
	nlh = nlmsg_put(skb,0,0,0,size - sizeof(struct nlmsghdr),0);
	old_tail = skb->tail;
	memcpy(NLMSG_DATA(nlh),buf,strlen(buf)); //填充数据区
	nlh->nlmsg_len = (skb->tail) - old_tail; //设置消息长度
	
	//设置控制字段
	NETLINK_CB(skb).pid = 0;
	NETLINK_CB(skb).dst_group = 0;
	printk("skb->data:%s\n",(char *)NLMSG_DATA((struct nlmsghdr *)skb->data));
	//发送数据
	ret = netlink_unicast(nl_fd,skb,user_process.pid,MSG_DONTWAIT);
	printk("netlink_unicast return :%d\n",ret);
	return 0;
}

static void kernel_receive(struct sk_buff *skb_1)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh = NULL;
	char *data = "this is a test message from kernel!";
	printk("begin kernel_receive!\n");
	skb = skb_get(skb_1);
	
	if(skb->len >= sizeof(struct nlmsghdr)){
		nlh = (struct nlmsghdr *)skb->data;
		if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))&&(skb_1->len >= nlh->nlmsg_len)){
		user_process.pid = nlh->nlmsg_pid;
		printk("data receive from user are:%s\n",(char *)NLMSG_DATA(nlh));
	
		 printk("user_pid:%d\n",user_process.pid);
		send_to_user(data);
		}
	}else{
		 printk("data receive from user are:%s\n",(char *)NLMSG_DATA(nlmsg_hdr(skb_1)));
		send_to_user(data);	
	}
	kfree_skb(skb);
}

int __init test_netlink_init(void)
{
	nl_fd = netlink_kernel_create(&init_net,NETLINK_TEST,0,kernel_receive,NULL,THIS_MODULE);
	if(!nl_fd){
	printk("cannot create a netlink socket\n");
	return -1;
	}
	return 0;
}
void __exit test_netlink_exit(void)
{
	sock_release(nl_fd->sk_socket);
	printk("module exit!!\n");
}
module_init(test_netlink_init);
module_exit(test_netlink_exit);
