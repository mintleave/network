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
    //1.创建用于通信的客户端套接字文件描述符
    int cfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (cfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success cfd = %d\n", cfd);

    // 判断文件是否存在
    if (access("./linux", F_OK) == 0)
    {
        if (unlink("./linux") == -1) {
            perror("unlink error");
            return -1;
        }
    }

    //2.1 填充要绑定的地址信息结构体
    struct sockaddr_un cun;
    cun.sun_family = AF_UNIX;
    strcpy(cun.sun_path, "./linux");
    //2.2  绑定工作
    if (bind(cfd, (struct sockaddr*)&cun, sizeof(cun)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    // 3.数据收发
    char wbuf[128] = "";
    //填充服务器的地址信息结构体
    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, "./unix");

    while (1)
    {
        //清空容器
        bzero(wbuf, sizeof(wbuf));
        //从终端读取数据
        fgets(wbuf, sizeof(wbuf), stdin);
        wbuf[strlen(wbuf)-1] = 0;
        //将数据发送给服务器
        sendto(cfd, wbuf, strlen(wbuf), 0, (struct sockaddr*)&sun, sizeof(sun));
        //接收服务器发来的数据
        recvfrom(cfd, wbuf, sizeof(wbuf), 0, NULL, NULL);

        printf("服务器发来的消息为：%s\n", wbuf);
    }
    
    //4.关闭套接字
    close(cfd);
    return 0;
}
