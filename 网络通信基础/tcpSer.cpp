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
    /* bind函数
    #include <sys/types.h>
    #include <sys/socket.h>
    
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    功能：为套接字分配名称，给套接字绑定ip地址和端口号
    参数1：要被绑定的套接字文件描述符
    参数2：通用地址信息结构体，对于不同的通信域而言，使用的实际结构体是不同的，该结构体的目
    的是为了强制类型转换，防止警告
        通信域为：AF_INET而言，ipv4的通信方式
        struct sockaddr_in {
            sa_family_t sin_family;     // 地址族: AF_INET 
            in_port_t sin_port;         // 端口号的网络字节序 
            struct in_addr sin_addr;    // 网络地址 
        };

        // Internet address
        struct in_addr {
            uint32_t s_addr;        // ip地址的网络字节序 
        };

        通信域为：AF_UNIX而言，本地通信
        struct sockaddr_un {
            sa_family_t sun_family;         // 通信域：AF_UNIX 
            char sun_path[UNIX_PATH_MAX];   // 通信使用的文件 
        };
    参数3：参数2的大小
    返回值：成功返回0，失败返回-1并置位错误码*/

    /* listen函数
    #include <sys/types.h>
    #include <sys/socket.h>
    
    int listen(int sockfd, int backlog);
    功能：将套接字设置成被动监听状态
    参数1：套接字文件描述符
    参数2：挂起队列能够增长的最大长度，一般为128
    返回值：成功返回0，失败返回-1并置位错误码*/

    /* accept函数
    #include <sys/types.h>
    #include <sys/socket.h>
    
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    功能：阻塞等待客户端的连接请求，如果已连接队列中有客户端，则从连接队列中拿取第一个，并创
    建一个用于通信的套接字
    参数1：服务器套接字文件描述符
    参数2：通用地址信息结构体，用于接受已连接的客户端套接字地址信息的
    参数3：接收参数2的大小
    返回值：成功发那会一个新的用于通信的套接字文件描述符，失败返回-1并置位错误码*/

    /* recv,send数据收发
    #include <sys/types.h>
    #include <sys/socket.h>
    
    ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    功能: 从套接字中读取消息放入到buf中 
    参数1：通信的套接字文件描述符
    参数2：要存放数据的起始地址
    参数3：读取的数据的大小
    参数4：读取标识位，是否阻塞读取
            0：表示阻塞等待
            MSG_DONTWAIT：非阻塞
    返回值：可以是大于0：表示成功读取的字节个数
            可以是等于0：表示对端已经下线（针对于TCP通信）
            -1：失败，并置位错误码
            
    #include <sys/types.h>
    #include <sys/socket.h>
    
    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    功能：向套接字文件描述符中将buf这个容器中的内容写入
    参数1：通信的套接字文件描述符
    参数2：要发送的数据的起始地址
    参数3：发送的数据的大小
    参数4：读取标识位，是否阻塞读取
            0：表示阻塞等待
            MSG_DONTWAIT：非阻塞
    返回值：成功返回发送字节的个数
            -1：失败，并置位错误码*/

    /* close关闭套接字
    #include <unistd.h>
    int close(int fd);
    功能：关闭套接字文件描述符
    参数：要关闭的套接字文件描述符
    返回值：成功返回0，失败返回-1并置位错误码*/

    /* connect连接函数
    #include <sys/types.h>
    #include <sys/socket.h>
    
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    功能：将指定的套接字，连接到给定的地址上
    参数1：要连接的套接字文件描述符
    参数2：通用地址信息结构体
    参数3：参数2的大小
    返回值：成功返回0，失败返回-1并置位错误码*/

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
        printf("[%s:%d]:\n",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port));
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