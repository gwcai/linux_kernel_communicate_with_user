#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>

#define NETLINK_TEST 17
#define MSG_LEN 100

struct sockaddr_nl local;
struct sockaddr_nl kaddr;
int skfd;

struct msg_to_kernel
{
        struct nlmsghdr hdr;
	char data[MSG_LEN];
};
struct u_packet
{
	struct nlmsghdr hdr;
        char msg[MSG_LEN];
}u_msg;

int init_sock()
{
	skfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
        if(skfd < 0){
            printf("can not create a netlink socket\n");
            return -1;
        }
        memset(&local, 0, sizeof(local));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid();
        local.nl_groups = 0;
        if(bind(skfd, (struct sockaddr *)&local, sizeof(local)) != 0){
            printf("bind() error\n");
            return -1;
        }
        memset(&kaddr, 0, sizeof(kaddr));
        kaddr.nl_family = AF_NETLINK;
        kaddr.nl_pid = 0;
        kaddr.nl_groups = 0;
}

int send_to_kernel(int skfd,char *buf)  //发送信息到内核进程
{
	struct nlmsghdr *message;
        int ret;
        message = (struct nlmsghdr *)malloc(1);
	
	memset(message, '\0', sizeof(struct nlmsghdr));
        message->nlmsg_len = NLMSG_SPACE(strlen(buf));
        message->nlmsg_flags = 0;
        message->nlmsg_type = 0;
        message->nlmsg_seq = 0;
        message->nlmsg_pid = local.nl_pid;

	memcpy(NLMSG_DATA(message), buf, strlen(buf));

        printf("message sendto kernel are:%s, len:%d\n", (char *)NLMSG_DATA(message), message->nlmsg_len);
        ret = sendto(skfd, message, message->nlmsg_len, 0,(struct sockaddr *)&kaddr, sizeof(kaddr));

	return ret;
}
int receive_message(int skfd,char* msg)
{
	int ret;
	int len = sizeof(struct sockaddr_nl);
	 //接受内核态确认信息
        ret = recvfrom(skfd, &u_msg, sizeof(struct u_packet),0, (struct sockaddr*)&kaddr,&len);
	memcpy(msg,u_msg.msg,strlen(u_msg.msg));
	return ret;
}
int main(int argc, char* argv[])
{
	char *data = "This message is from eric's space";
	char msg[100];
        //初始化
        int ret1,ret2;
	int i = 3;
	init_sock();

	while(i > 0)
	{
		i--;
        	ret1 = send_to_kernel(skfd,data);
        	if(!ret1){
            		perror("send pid:");
            		exit(-1);
		}

        	//接受内核态确认信息
        	ret2 = receive_message(skfd,msg);
       		if(!ret2){
           		perror("recv form kerner:");
            		exit(-1);
		}
        	printf("message receive from kernel:%s\n",msg);
	}
        
	close(skfd);
        return 0;
}
