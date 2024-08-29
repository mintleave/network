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
    /* 组播
    1.广播是给同一个网络下的所有主机发送消息，会占用大量的网络带宽，影响正常网络通信，造成网络拥塞
    2.组播也是实现一对多的通信机制，也就是说，加入同一个多播组的成员都能收到组播消息
    3.组播也是使用UDP实现的，发送者发送的消息，无论接收者愿不愿意，都会收到消息
    4.组播地址：D类网络地址 （224.0.0.0 --- 239.255.255.255）*/

     //1.创建用于通信的套接字文件描述符
    int sndfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sndfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sndfd);

    //2.绑定ip地址和端口号(可选)

    //3.数据收发
    char wbuf[128] = "";
    // 填充接收端的地址信息结构体
    struct sockaddr_in rin;
    rin.sin_family = AF_INET;
    rin.sin_port = htons(8888);
    rin.sin_addr.s_addr = inet_addr("224.1.2.3");
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
