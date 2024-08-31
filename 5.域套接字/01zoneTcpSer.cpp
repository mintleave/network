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
    // 1.创建用于连接的套接字文件描述符
    //参数1：AF_UNIX表示本地通信，同一主机的多进程通信
    //参数2：SOCK_STREAM表示使用的是tcp通信
    //参数3：由于参数2指定了协议，参数3填0即可
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sfd);

    /*如何判断一个文件是否存在于文件系统
    #include <unistd.h>
    int access(const char *pathname, int mode);
    功能：判断给定的文件是否具有给的的权限
    参数1：要被判断的文件描述符
    参数2：要被判断的权限
        R_OK:读权限
        W_OK:写权限
        X_OK:执行权限
        F_OK:是否存在
    返回值：如果要被判断的权限都存在，则返回0，否则返回-1并置位错误码*/

    /*如何删除一个文件系统中的文件
    #include <unistd.h>
    int unlink(const char *path);
    功能：删除指定的文件
    参数：要删除的文件路径
    返回值：成功删除返回0，失败返回-1并置位错误码*/

    // 判断套接字文件是否存在
    if (access("./unix", F_OK) == 0) {
        // 说明文件已经存在，需要将其进行删除
        if (unlink("./unix") == -1) {
            perror("ulink error");
            return -1;
        }
    }

    //2.绑定ip地址和端口号
    //2.1 填充要绑定的ip地址和端口号结构体
    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, "./unix");
    //2.2  绑定工作
    //参数1：要被绑定的套接字文件描述符
    //参数2：要绑定的地址信息结构体，需要进行强制类型转换，防止警告
    //参数3：参数2的大小
    if (bind(sfd, (struct sockaddr*)&sun, sizeof(sun)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success!\n");

    //3.启动监听
    //参数1：要启动监听的文件描述符
    //参数2：挂起队列的长度
    if (listen(sfd, 128) == -1)
    {
        perror("listen error\n");
        return -1;
    }
    printf("listen success\n");

    //4.阻塞等待客户端的连接请求
    //定义变量，用于接受客户端地址信息结构体
    struct sockaddr_un cun;
    socklen_t socklen = sizeof(cun); 

    //参数1：服务器套接字文件描述符
    //参数2：用于接收客户端地址信息结构体的容器，如果不接收，也可以填NULL
    //参数3：接收参数2的大小，如果参数2为NULL，则参数3也是NULL
    int newfd = accept(sfd, (struct sockaddr *)&cun, &socklen);
    if (newfd == -1)
    {
        perror("accept error");
        return -1;
    }         
    printf("[%s]:已连接成功!!!!\n", cun.sun_path); 

    //5.数据收发
    char rbuf[128] = "";
    while(1) {
        //清空容器中的内容
        bzero(rbuf, sizeof(rbuf));
        //从套接字中读取消息
        int res = recv(newfd, rbuf, sizeof(rbuf), 0);
        if (res == 0)
        {
            printf("对端已经下线\n");
            break;
        }
        printf("[%s]:%s\n", cun.sun_path, rbuf);
        //对收到的数据处理一下，回给客户端
        strcat(rbuf, "^-^");
        //将消息发送给客户端
        if (send(newfd, rbuf, strlen(rbuf), 0) == -1)
        {
            perror("send error");
            return -1;
        }
        printf("发送成功\n");
    }
    //6.关闭套接字
    close(newfd);
    close(sfd);

    return 0;
}
