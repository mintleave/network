#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SER_PORT 8888
#define SER_IP   "192.168.88.142"

int main(int argc, char const *argv[])
{
    /* epoll
    1、sellect和poll都是基于线性结构进行检测集合，而epoll是基于树形结构（红黑树）完成管理检测集合的
    2、select和poll检测时，随着集合的增大，效率会越来越低。epoll使用的是函数回调机制，效率较高。
    处理文件描述符的效率也不会随着文件秒数的增大而降低。
    3、select和poll在工作过程中，不断的在内核空间与用户空间频繁拷贝文件描述符的数据。epoll在注册
    新的文件描述符或者修改文件描述符时，只需进行一次，能够有效减少数据在用户空间和内核空间之间的切换
    4、和poll一样，epoll没有最大文件描述符的限制，仅仅收到程序能够打开的最大文件描述符数量限制
    5、对于select和poll而言，需要对返回的文件描述符集合进行判断后才知道时哪些文件描述符就绪了，而
    epoll可以直接得到已经就绪的文件描述符，无需再次检测
    6、当多路复用比较频发进行、IO流量频繁的时候，一般不使用select和poll，使用epoll比较合适
    7、epoll只适用于linux平台，不能跨平台操作*/

    /* 相关API
    #include <sys/epoll.h>
    int epoll_creat(int size);
    功能：创建一个epoll实例，并返回该实例的句柄，是一个文件描述符
    参数1：epoll实例中能够容纳的最大节点个数，自从linux 2.6.8版本后，size可以忽略，但是
    必须要是一个大于0的数字
    返回值：成功返回控制epoll实例的文件描述符，失败返回-1并置位错误码
    
    #include <sys/epoll.h>
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    功能：完成对epoll实例的控制
    参数1：通过epoll_create创建的epoll实例文件描述符
    参数2：op表示要进行的操作
        EPOLL_CTL_ADD：向epoll树上添加新的要检测的文件描述符
        EPOLL_CTL_MOD：改变epoll树上的文件描述符检测的事件
        EPOLL_CTL_DEL：删除epoll树上的要检测的文件描述符,此时参数3可以省略填NULL
    参数3：要检测的文件描述符
    参数4：要检测的事件，是一个结构体变量地址，属于输入变量
        typedef union epoll_data {
            void *ptr;      //提供的解释性数据
            int fd;         //文件描述符（常用）
            uint32_t u32;
            uint64_t u64;
        } epoll_data_t;
        
        struct epoll_event {
            uint32_t events;       // 要检测的事件 
            epoll_data_t data;     // 用户有效数据，是一个共用体 
        };
        
        要检测的事件：
            EPOLLIN：读事件
            EPOLLOUT：写事件
            EPOLLERR：异常事件
            EPOLLET：表示设置epoll的模式为边沿触发模式
            
    #include <sys/epoll.h>
    int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
    功能：阻塞检测epoll实例中是否有文件描述符准备就绪，如果准备就绪了，就解除阻塞
    参数1：epoll实例对于的文件描述符
    参数2：文件描述符集合，当有文件描述符产生事件时，将所有产生事件的文件描述符，放入到该集合中
    参数3：参数2的大小
    参数4：超时时间，以毫秒为单位的超时时间，如果填-1表示永久阻塞
    返回值：
        >0:表示解除本次操作的触发的文件描述符个数
        =0：表示超时，但是没有文件描述符产生事件
        =-1：失败，置位错误码*/

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

    // 创建epoll实例，用于检测文件描述符
    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll_create error");
        return -1;
    }
    
    // 将sfd放入到检测集合中
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sfd;
    // 功能：将sfd放入到检测集合中
    // 参数1：epoll实例的文件描述符
    // 参数2：epoll操作，表示要添加文件描述符
    // 参数3：要检测的文件描述符的值
    // 参数4：要检测的事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev);

    // 定义接收返回的事件集合
    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(evs[0]);

    while (1)
    {
        // 阻塞检测文件描述符集合中是否有事件产生
        // 参数1：epoll实例的文件描述符
        // 参数2：返回触发事件的文件事件集合
        // 参数3：集合的大小
        // 参数4：是否阻塞
        int num = epoll_wait(epfd, evs, size, -1);
        printf("num = %d\n", num); // 输出本次触发的文件描述符个数

        for (int i = 0; i < num; i++)
        {
            int fd = evs[i].data.fd;

            // 判断是否为sfd文件描述符就绪
            if (fd == sfd)
            {
                // 说明有新的客户端发来连接请求
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

                // 将客户端文件描述符放入到epoll检测集合中
                struct epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = newfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &ev);
            }
            else 
            {
                // 表示客户端文件描述符就绪，也就是说客户端有数据发来
                //5.数据收发
                char rbuf[128] = "";
                
                //清空容器中的内容
                bzero(rbuf, sizeof(rbuf));
                //从套接字中读取消息
                int res = recv(fd, rbuf, sizeof(rbuf), 0);
                if (res == 0)
                {
                    printf("对端已经下线\n");
                    //将客户端从epoll树中删除
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    break;
                }
                printf("收到数据:%s\n", rbuf);
                //对收到的数据处理一下，回给客户端
                strcat(rbuf, "^-^");
                //将消息发送给客户端                    
                if (send(fd, rbuf, strlen(rbuf), 0) == -1)
                {
                    perror("send error");
                    return -1;
                }
                printf("发送成功\n");
            }
        }
    }
    
    //6.关闭套接字
    close(sfd);
    close(epfd);

    return 0;
}
