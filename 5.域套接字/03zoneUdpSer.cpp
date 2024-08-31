#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char const *argv[])
{
    //1.创建用于通信的套接字文件描述符
    int sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sfd);

    //2.绑定套接字文件
    //判断套接字文件是否存在
    if (access("./unix", F_OK) == 0) {
        //说明文件已经存在，需要将其进行删除操作
        if (unlink("./unix") == -1) {
            perror("unlink error");
            return -1;
        }
    }

    //2.绑定ip地址和端口号
    // 2.1填充要绑定的ip地址和端口号结构体
    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, "./unix");
    // 2.2绑定工作
    if (bind(sfd, (struct sockaddr*)&sun, sizeof(sun)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    //3.数据收发
    char rbuf[128] = "";
    //定义容器接收对端的地址信息结构体
    struct sockaddr_un cun;
    socklen_t socklen = sizeof(cun);

    while (1)
    {
        //清空容器
        bzero(rbuf, sizeof(rbuf));
        //从客户端中读取消息
        if (recvfrom(sfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&cun,
        &socklen) == -1)
        {
            perror("recvfrom error");
            return -1;
        }
        printf("[%s]:%s\n", cun.sun_path, rbuf);
        //加个笑脸发给客户端
        strcat(rbuf, "^-^");
        //将数据发送给客户端
        sendto(sfd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&cun, sizeof(cun));

        printf("发送成功\n");
    }
    
    //4.关闭套接字
    close(sfd);
    return 0;
}
