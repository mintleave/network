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
    /* setsocketopt
    设置套接字加入多播组 在IPPROTO_IP网络层 IP_ADD_MEMBERSHIP属性
        属性值为结构体类型
        struct ip_mreqn {
            struct in_addr imr_multiaddr; // 组播地址的网络字节序 
            struct in_addr imr_address; // 接收端的主机地址 
            int imr_ifindex; // 网卡设备编号 一般为2 可以通过指令 ip ad 查看 
        };
    */

    //1.创建用于通信的客户端套接字文件描述符
    int recvfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (recvfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success cfd = %d\n", recvfd);

    // 将该套接字加入多播组
    struct ip_mreqn imr;
    imr.imr_multiaddr.s_addr = inet_addr("224.1.2.3");
    imr.imr_address.s_addr = inet_addr("192.168.88.142");
    imr.imr_ifindex = 2;
    //调用设置网络属性函数
    if (setsockopt(recvfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)) == -1) {
        perror("setsockopt error");
        return -1;
    }
    printf("成功加入多播组\n");

    //2.绑定ip地址和端口号
    // 2.1填充要绑定的地址信息结构体
    struct sockaddr_in rin;
    rin.sin_family = AF_INET;
    rin.sin_port = htons(8888);
    rin.sin_addr.s_addr = inet_addr("224.1.2.3");
    // 2.2绑定工作
    if (bind(recvfd, (struct sockaddr*)&rin, sizeof(rin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    //3.数据收发
    char rbuf[128] = "";
    while (1)
    {
        // 清空容器
        bzero(rbuf, sizeof(rbuf));
        // 从客户端中读取消息
        if (recvfrom(recvfd, rbuf, sizeof(rbuf), 0, NULL, NULL) == -1)
        {
            perror("recvfrom error");
            return -1;
        }
        printf("[读取的消息为]:%s\n", rbuf);
    }
    //4.关闭套接字
    close(recvfd);
    return 0;
}
