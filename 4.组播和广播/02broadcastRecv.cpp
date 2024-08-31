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
    //1.创建用于通信的客户端套接字文件描述符
    int recvfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (recvfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success cfd = %d\n", recvfd);

    //2.绑定ip地址和端口号
    // 2.1填充要绑定的地址信息结构体
    struct sockaddr_in rin;
    rin.sin_family = AF_INET;
    rin.sin_port = htons(8888);
    rin.sin_addr.s_addr = inet_addr("192.168.88.255");
    // 2.2绑定工作
    if (bind(recvfd, (struct sockaddr*)&rin, sizeof(rin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    //3.数据收发
    char rbuf[128] = "";
    //定义容器接受对端的地址信息结构体
    struct sockaddr_in cin;
    socklen_t socklen = sizeof(cin);

    while (1)
    {
        // 清空容器
        bzero(rbuf, sizeof(rbuf));
        // 从客户端中读取消息
        if (recvfrom(recvfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&cin, &socklen) == -1)
        {
            perror("recvfrom error");
            return -1;
        }
        printf("[%s:%d]:%s\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port),rbuf);
    }
    
    //4.关闭套接字
    close(recvfd);
    return 0;
}
