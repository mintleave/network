#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SER_PORT 8888
#define SER_IP   "192.168.88.142"

int main(int argc, char const *argv[])
{
    /* select函数
    #include <sys/select.h>
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout);
    功能：阻塞等待文件描述符集合中是否有事件产生，如果有事件产生，则解除阻塞
    参数1：文件描述符集合中，最大的文件描述符 加1
    参数2、参数3、参数4：分别表示读集合、写集合、异常处理集合的起始地址
        由于对于写操作而言，我们也可以转换读操作，所以，只需要使用一个集合就行
        对于不使用的集合而言，直接填NULL即可
    参数5：超时时间，如果填NULL表示永久等待，如果想要设置时间，需要定义一个如下结构体类型的变量，并将地址传递进去
        struct timeval {
            long tv_sec; // 秒数 
            long tv_usec; // 微秒 
        };
        
        struct timespec {
            long tv_sec; // 秒数 
            long tv_nsec; // 纳秒 
        };
    返回值：
        >0:成功返回解除本次阻塞的文件描述符的个数
        =0:表示设置的超时时间，时间已经到达，但是没有事件事件产生
        =-1：表示失败，置位错误码
    注意：当该函数解除阻塞时，文件描述符集合中，就只剩下本次触发事件的文件描述符，其余的文
    件描述符就被删除了*/

    /* 专门针对于文件描述符集合提供的函数
    void FD_CLR(int fd, fd_set *set);   //将fd文件描述符从容器set中删除
    int  FD_ISSET(int fd, fd_set *set);  //判断fd文件描述符，是否存在于set容器中
    void FD_SET(int fd, fd_set *set);   //将fd文件描述符，放入到set容器中
    void FD_ZERO(fd_set *set);          //清空set容器 */

    // 1.创建用于连接的套接字文件描述符
    //参数1：AF_INET表示使用的是ipv4的通信协议
    //参数2：SOCK_STREAM表示使用的是tcp通信
    //参数3：由于参数2指定了协议，参数3填0即可
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sfd);

    //2.绑定ip地址和端口号
    //2.1 填充要绑定的ip地址和端口号结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;           //通信域
    sin.sin_port   = htons(SER_PORT);   //端口号
    sin.sin_addr.s_addr = inet_addr(SER_IP);
    //2.2  绑定工作
    //参数1：要被绑定的套接字文件描述符
    //参数2：要绑定的地址信息结构体，需要进行强制类型转换，防止警告
    //参数3：参数2的大小
    if (bind(sfd, (struct sockaddr*)&sin, sizeof(sin)) == -1)
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
    struct sockaddr_in cin;
    socklen_t socklen = sizeof(cin);

    //定义文件描述符集合
    fd_set readfds, tempfds;

    //将该文件描述符集合清空
    FD_ZERO(&readfds);

    //将0号文件描述符以及sfd文件描述符放入到集合中
    FD_SET(0, &readfds);
    FD_SET(sfd, &readfds);

    //定义一个变量，用于存储容器中的最大文件描述符
    int maxfd = sfd;

    // 接收客户端连接请求后，创建的通信套接字文件描述符
    int newfd = -1;
    //定义一个地址信息结构体数组来存储客户端对应的地址信息
    struct sockaddr_in cin_arr[1024];
    while (1)
    {
        // 将reafds备份一份放入tempfds中
        tempfds = readfds;
        // 调用阻塞函数，完成对文件描述符集合的管理工作
        int res = select(maxfd+1, &tempfds, NULL, NULL, NULL);
        if (res == -1)
        {
            perror("select error");
            return -1;
        }
        else if (res == 0)
        {
            printf("time out !!!\n");
            return -1;
        }

        // 程序执行至此，表示一定有其中至少一个文件描述符产生了事件，只需要判断哪个文件描述符还在集合中
        // 就说明该文件描述符产生了事件
        // 表示sfd文件描述符触发了事件
        if (FD_ISSET(sfd, &tempfds))
        {
            //参数1：服务器套接字文件描述符
            //参数2：用于接收客户端地址信息结构体的容器，如果不接收，也可以填NULL
            //参数3：接收参数2的大小，如果参数2为NULL，则参数3也是NULL
            int newfd = accept(sfd, (struct sockaddr *)&cin, &socklen);
            if (newfd == -1)
            {
                perror("accept error");
                return -1;
            }
            printf("[%s:%d]:连接成功\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
            //将该客户端对应的套接字地址信息结构体放入数组对应的位置上
            cin_arr[newfd] = cin;
            //将当前的newfd放入到检测文件描述符集合中，以便于检测使用
            FD_SET(newfd, &readfds);
            if (maxfd < newfd)
            {
                maxfd = newfd;
            }
        }

        // 判断0号文件描述符是否产生了事件
        if (FD_ISSET(0, &tempfds))
        {
            //5.数据收发
            char wbuf[128] = "";
            // 从终端读取数据,阻塞函数
            fgets(wbuf, sizeof(wbuf), stdin);
            printf("触发了键盘输入事件：%s\n", wbuf);
            //将数据发送给所有客户端
            for (int i = 4; i <= maxfd; i++)
            {
                send(i, wbuf, strlen(wbuf), 0);
            }
        }
        // 判断是否是newfd产生了事件
        // 循环将所有客户端文件描述符遍历一遍，如果还存在于tempfds中的客户端，表示有数据接收过来
        for (int i = 4; i <= maxfd; i++)
        {
            if (FD_ISSET(i, &tempfds))
            {
                // 数据容器
                char rbuf[128] = "";
                // 清空容器中的内容
                bzero(rbuf, sizeof(rbuf));
                // 从套接字中读取消息
                int res = recv(i, rbuf, sizeof(rbuf), 0);
                if (res == 0)
                {
                    printf("对端已经下线\n");
                    // 将文件描述符进行关闭
                    close(i);
                    // 需要将该文件描述符从readfds中删除
                    FD_CLR(i, &readfds);

                    // 更新maxfd
                    for (int i = maxfd; i > 0; i--)
                    {
                        if (FD_ISSET(i, &readfds))
                        {
                            maxfd = i;
                            break;
                        }
                    }
                    continue;
                }
                printf("[%s:%d]:%s\n", inet_ntoa(cin_arr[i].sin_addr),ntohs(cin_arr[i].sin_port), rbuf);

                // 对收到的数据处理一下，回给客户端
                strcat(rbuf, "^-^");
                // 将消息发送给客户端
                if (send(i, rbuf, strlen(rbuf), 0) == -1)
                {
                    perror("send error");
                    return -1;
                }
                printf("发送成功\n");
            }
        }
    }
    close(sfd);

    return 0;
}
