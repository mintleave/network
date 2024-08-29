#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{
    /* 网络属性
    #include <sys/types.h>
    #include <sys/socket.h>
    
    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
    int setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen);
    功能：设置或者获取套接字文件描述符的属性
    参数1：套接字文件描述符
    参数2：表示操作的套接字层
        SOL_SOCKET:表示应用层或者套接字层，通过man 7 socket进行查找
            该层中常用的属性：SO_REUSEADDR 地址快速重用
                            SO_BROADCAST 允许广播
                            SO_RCVTIMEO and SO_SNDTIMEO:发送或接收超时时间
        IPPROTO_TCP:表示传输层的基于TCP的协议
        IPPROTO_UDP:表示传输层中基于UDP的协议
        IPPROTO_IP:表示网络层
            该层常用的属性：IP_ADD_MEMBERSHIP，加入多播组
    参数3:对应层中属性的名称
    参数4：参数3属性的值，一般为int类型，其他类型，会给出
    参数5：参数4的大小
    返回值：成功返回0，失败返回-1并置位错误码*/

    /* 广播
    1.主机之间是一对多的通信方式，网络对其中的每一台主机都会将消息进行转发
    2.在当前网络下的所有主机都会收到该消息，无论是否愿意收到消息
    3.基于无连接的通信方式完成，UDP通信
    4.广播分为发送端和接收端
    5.对于发送端而言，需要设置允许广播，并且向广播地址发送数据
    6.广播地址：网络号 + 全为1的主机号，可以通过指令 ifconfig查看
    7.广播消息不能穿过路由器到骨干路由上*/

    //1.创建用于通信的套接字文件描述符
    int sndfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sndfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sndfd);

    //设置允许广播
    int broad = 1;
    if (setsockopt(sndfd, SOL_SOCKET, SO_BROADCAST, &broad, sizeof(broad)) == -1) {
        perror("setsockopt error");
        return -1;
    }
    printf("成功设置广播\n");

    //2.绑定ip地址和端口号 (可以绑定也可以不绑定)

    //3.数据收发
    char wbuf[128] = "";
    // 填充接收端的地址信息结构体
    struct sockaddr_in rin;
    rin.sin_family = AF_INET;
    rin.sin_port = htons(8888);
    rin.sin_addr.s_addr = inet_addr("192.168.88.255");

    while (1)
    {
        // 清空容器
        bzero(wbuf, sizeof(wbuf));

        // 从终端读取数据
        fgets(wbuf, sizeof(wbuf), stdin);
        wbuf[strlen(wbuf)-1] = 0;

        // 将数据发送给服务端
        sendto(sndfd, wbuf, strlen(wbuf), 0, (struct sockaddr*)&rin, sizeof(rin));

        printf("发送成功\n");
    }

    //4.关闭套接字
    close(sndfd);
    
    return 0;
}