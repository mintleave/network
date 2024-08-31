#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SER_PORT 8888
#define SER_IP   "192.168.88.142"

int main(int argc, char const *argv[])
{
    /* ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr
       *src_addr, socklen_t *addrlen);
       功能：从套接字中读取消息放入到buf中，并接受对端的地址信息结构体
       参数1：套接字文件描述符
       参数2：存放数据的容器起始地址
       参数3：读取的数据大小
       参数4：是否阻塞，0表示阻塞 ， MSG_DONTWAIT表示非阻塞
       参数5：接收对端地址信息结构体的容器
       参数6：参数5的大小
       返回值：成功返回读取字节的个数，失败返回-1并置位错误码*/

    /* ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct
       sockaddr *dest_addr, socklen_t addrlen);
       功能：向套接字中发送消息，并且指定对方的地址信息结构体
       参数1：套接字文件描述符
       参数2：要发送的数据的起始地址
       参数3：要发送的数据大小
       参数4：是否阻塞，0表示阻塞 ， MSG_DONTWAIT表示非阻塞
       参数5：要发送的对端地址信息结构体
       参数6：参数5的大小
       返回值：成功返回发送的字节的个数，失败返回-1并置位错误码*/

    //1.创建用于通信的套接字文件描述符
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sfd);

    //2.绑定ip地址和端口号
    // 2.1填充要绑定的ip地址和端口号结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SER_PORT);
    sin.sin_addr.s_addr = inet_addr(SER_IP);
    // 2.2绑定工作
    if (bind(sfd, (struct sockaddr*)&sin, sizeof(sin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");
    
    //3.数据收发
    char rbuf[128] = "";
    //定义容器接收对端的地址信息结构体
    struct sockaddr_in cin;
    socklen_t socklen = sizeof(cin);
    
    while (1)
    {
        //清空容器
        bzero(rbuf, sizeof(rbuf));
        //从客户端中读取消息
        if (recvfrom(sfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&cin,
        &socklen) == -1)
        {
            perror("recvfrom error");
            return -1;
        }
        printf("[%s:%d]:%s\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), rbuf);
        //加个笑脸发给客户端
        strcat(rbuf, "^-^");
        //将数据发送给客户端
        sendto(sfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&cin, sizeof(cin));

        printf("发送成功\n");
    }
    
    //4.关闭套接字
    close(sfd);

    return 0;
}
