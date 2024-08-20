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
#define CLI_PORT 9999
#define CLI_IP   "192.168.88.142"

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

    //1.创建用于通信的客户端套接字文件描述符
    int cfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (cfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success cfd = %d\n", cfd);

    //2.绑定ip地址和端口号（可选）
    // 2.1填充要绑定的地址信息结构体
    struct sockaddr_in cin;
    cin.sin_family = AF_INET;
    cin.sin_port   = htons(CLI_PORT);
    cin.sin_addr.s_addr = inet_addr(CLI_IP);
    // 2.2绑定工作
    if (bind(cfd, (struct sockaddr*)&cin, sizeof(cin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    //3.数据收发
    char wbuf[128] = "";
    //填充服务器的地址信息结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SER_PORT);
    sin.sin_addr.s_addr = inet_addr(SER_IP);

    while (1)
    {
        //清空容器
        bzero(wbuf, sizeof(wbuf));
        //从终端读取数据
        fgets(wbuf, sizeof(wbuf), stdin);
        wbuf[strlen(wbuf)-1] = 0;
        //将数据发送给服务器
        sendto(cfd, wbuf, strlen(wbuf), 0, (struct sockaddr*)&sin, sizeof(sin));
        //接收服务器发来的数据
        recvfrom(cfd, wbuf, sizeof(wbuf), 0, NULL, NULL);

        printf("服务器发来的消息为：%s\n", wbuf);
    }
    
    //4.关闭套接字
    close(cfd);
    
    return 0;
}
