#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define SER_PORT    8888
#define SER_IP      "192.168.88.142"
#define CLI_PORT    9999
#define CLI_IP      "192.168.88.142"

int main(int argc, char const *argv[])
{
    /* poll函数
    #include <poll.h>
    int poll(struct pollfd* fds, nfds_t nfds, int timeout);
    功能：阻塞等待文件描述符集合中是否有事件产生，如果有，则解除阻塞，返回本次触发事件的文件描述符个数
    参数1：文件描述符集合容器的起始地址，是一个结构体数组，结构体类型如下
        struct pollfd {
            int fd;         // 文件描述符 
            short events;   // 要等待的事件：由用户填写 
            short revents;  // 实际发生的事件 ：调用函数结束后，内核会自动设置
        };
        关于事件对应的位：
        POLLIN:读事件
        POLLOUT：写事件
    参数2：集合中文件描述符的个数
    参数3：超时时间，负数表示永久等待，0表示非阻塞
    返回值：
        >0:表示触发本次解除阻塞事件的文件描述符的个数
        =0:表示超时
        =-1:出错，置位错误码
    */

    //1.创建用于通信的客户端套接字文件描述符
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success cfd = %d\n", cfd);

    //2.绑定ip地址和端口号(可选)
    //2.1 填充要绑定的地址信息结构体
    /*struct sockaddr_in cin;
    cin.sin_family = AF_INET;
    cin.sin_port   = htons(CLI_PORT);
    cin.sin_addr.s_addr = inet_addr(CLI_IP);*/
    //2.2  绑定工作
    /*if (bind(cfd, (struct sockaddr*)&cin, sizeof(cin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");*/
    
    //3.连接服务器
    //3.1 填充要连接的服务器地址信息结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SER_PORT);
    sin.sin_addr.s_addr = inet_addr(SER_IP);
    //3.2 连接工作
    if (connect(cfd, (struct sockaddr*)&sin, sizeof(sin)) == -1)
    {
        perror("connect error");
        return -1;
    }
    printf("连接服务器成功\n");

    // 使用poll完成终端输入和套接字接收数据的并发执行
    struct pollfd pfds[2];
    // 分别给数组中两个文件描述符成员赋值
    pfds[0].fd = 0;
    pfds[0].events = POLLIN;

    pfds[1].fd = cfd;
    pfds[1].events = POLLIN;

    //4.数据收发
    char wbuf[128] = "";
    while (1)
    {
        // 功能：阻塞等待文件描述符集合中是否有事件产生
        // 参数1：文件描述符集合起始地址
        // 参数2：文件描述符个数
        // 参数3：表示永久等待
        int res = poll(pfds, 2, -1);
        if (res == -1)
        {
            perror("poll errro");
            return -1;
        }
        
        /*程序执行至此，表示文件描述符容器中，有事件产生*/
        // 表示0号文件描述符的事件
        if (pfds[0].revents == POLLIN)
        {
            //清空容器
            bzero(wbuf, sizeof(wbuf));

            //从终端获取数据
            fgets(wbuf, sizeof(wbuf), stdin);
            wbuf[strlen(wbuf)-1] = 0;

            //将数据发送给服务器
            if (send(cfd, wbuf, sizeof(wbuf), 0) == -1)
            {
                perror("send error");
                return -1;
            }
        }
        
        //表示有服务端发来消息
        if (pfds[1].revents == POLLIN)
        {
            //接受服务器发送过来的消息
            if (recv(cfd, wbuf, sizeof(wbuf), 0) == 0)
            {
                printf("对端已经下线\n");
                break;
            }
            printf("收到服务器消息为：%s\n", wbuf);
        }
    }
    
    //5.关闭套接字
    close(cfd);

    return 0;
}
