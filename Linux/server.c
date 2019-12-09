#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SIZE 128
#define PORT 1847

#if 0
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

功能：
监视文件描述符对应的事件是否准备好
参数:
nfds 所有的监视文件描述中最大一个， 加1
readfds 读集合
writefds 写集合
execeptfds 异常集合
timeout 超时时间

返回值:
成功: 返回准备好的文件描述符个数
超时: 0
出错： -1

struct timeval结构体
struct timeval {
    long    tv_sec;         /* seconds */ 秒
        long    tv_usec;        /* microseconds */ 微秒
};


将文件描述符fd从集合set中移除
void FD_CLR(int fd, fd_set *set);
判断文件描述符fd是否在集合set中
int  FD_ISSET(int fd, fd_set *set);
将文件描述符fd添加到集合set中
void FD_SET(int fd, fd_set *set);
清空集合set
void FD_ZERO(fd_set *set);

#endif

int main(void)
{
    int ret = -1;
    int i = 0;
    int sockfd = -1;
    int connfd = -1;
    int maxfd = -1;
    int op = -1;

    struct sockaddr_in addr;
    struct sockaddr_in from;

    socklen_t len;

    fd_set readfds;
    fd_set savefds;

    struct timeval tmo;

    char buf[SIZE];

    //1. 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("socket"); 
        goto err0;
    }

    //设置端口复用
    op = 1;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &op, sizeof(op));
    if (-1 == ret)
    {
        perror("setsockopt"); 
        goto err1;
    }

    //2. 初始化结构体
    memset(&addr, 0, sizeof(addr));    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    //3. 绑定
    ret = bind(sockfd, (void*)&addr, sizeof(addr));
    if (-1 == ret)
    {
        perror("bind"); 
        goto err1;
    }

    //4. 监听
    ret = listen(sockfd, 10);
    if (-1 == ret)
    {
        perror("listen");     
        goto err1;
    }


    maxfd = sockfd + 1;
    //清空savefds
    FD_ZERO(&savefds);

    //添加需要监视文件描述符
    FD_SET(sockfd, &savefds);
    FD_SET(STDIN_FILENO, &savefds);



    //6. 循环接受客户端连接 接收和发送数据
    while(1)
    {

        //将需要监视文件描述符赋值给readfds
        readfds = savefds;

        //设置超时时间
        tmo.tv_sec = 3;
        tmo.tv_usec = 0;

        //有对应事件发生， 就立即返回
        //超时返回
        //出错返回
        ret = select(maxfd, &readfds, NULL, NULL, &tmo); 
        if (-1 == ret)
        {
            perror("select"); 
            break;
        }
        else if (0 == ret)
        {
            //超时
            //printf("3 seconds timeout....\n"); 
        }
        else
        {
            //准备好的文件描述符保存在readfds
            if (FD_ISSET(sockfd, &readfds))
            {
                //接受客户端连接
                memset(&from, 0, sizeof(from));    
                len = sizeof(from);
                connfd = accept(sockfd, (void*)&from, &len);
                if (-1 == connfd)
                {
                    perror("accept"); 
                    break;
                }

                printf("\033[31m客户端%s:%d建立连接..\033[0m\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                //将新的客户端加入到监听集合中
                FD_SET(connfd, &savefds);

                maxfd = maxfd <= connfd ? connfd + 1 : maxfd;
            }

            for (i = sockfd + 1; i < maxfd; i++)
            {
                //标准输入有数据读
                if(FD_ISSET(STDIN_FILENO, &readfds))
                {
                    memset(buf, 0, SIZE);
                    fgets(buf, SIZE, stdin);
                    if(strcmp(buf, "\n") == 0)
                    {
                        continue;
                    }
                    //发送给全部客户端
                    for(int j = i; j < maxfd; ++j)
                    {

                        ret = send(j, buf, strlen(buf) - 1, 0); 
                    }

                }
                //客户端有数据可以读
                if (FD_ISSET(i, &readfds))
                {
                    memset(buf, 0, SIZE); 
                    ret = recv(i, buf, SIZE, 0);
                    if (-1 == ret || 0 == ret)
                    {
                        perror("recv");  
                        //从集合中移除 
                        FD_CLR(i, &savefds);

                        if (i == maxfd - 1)
                        {
                            maxfd = maxfd - 1;
                        }

                        continue;
                    }
                    printf("客户端: %s\n", buf);

                    //新连接
                    if(strncmp(buf, "*##**#", 6) == 0)
                    {
                        char send_buf[50] = {0};
                        sprintf(send_buf, "欢迎%s!", buf+6);
                        send(i, send_buf, strlen(send_buf), 0); 
                    }

                    ////发送数据
                    //ret = send(i, buf, strlen(buf), 0); 
                    //if (ret <= 0)
                    //{
                    //    perror("send"); 

                    //    //从集合中移除 
                    //    FD_CLR(i, &savefds);

                    //    if (i == maxfd - 1)
                    //    {
                    //        maxfd = maxfd - 1;
                    //    }

                    //    continue;
                    //}
                } //if
            } //for

        }
    }

    //7. 断开连接
    //close(connfd);
    close(sockfd);

    return 0;
err1:
    close(sockfd);
err0:
    return 1;
}
