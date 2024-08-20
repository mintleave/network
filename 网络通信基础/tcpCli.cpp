#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SER_PORT 8888               //服务器端口号
#define SER_IP   "192.168.88.142"   //服务器IP地址
#define CLI_PORT 9999               //客户端端口号
#define CLI_IP   "192.168.88.142"   //客户端ip地址

int main(int argc, char const *argv[]) {

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
    struct sockaddr_in cin;
    cin.sin_family = AF_INET;
    cin.sin_port   = htons(CLI_PORT);
    cin.sin_addr.s_addr = inet_addr(CLI_IP);
    //2.2  绑定工作
    if (bind(cfd, (struct sockaddr*)&cin, sizeof(cin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");
    
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

    //4.数据收发
    char wbuf[128] = "";
    while (1)
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
        //接受服务器发送过来的消息
        if (recv(cfd, wbuf, sizeof(wbuf), 0) == 0)
        {
            printf("对端已经下线\n");
            break;
        }
        printf("收到服务器消息为：%s\n", wbuf);
    }
    
    //5.关闭套接字
    close(cfd);

    return 0;
}