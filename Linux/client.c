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


int main(void)
{
    int ret = -1;
    int sockfd = -1;
    int maxfd = -1;

    struct sockaddr_in addr;

    //select相关的参数
    fd_set readfds;
    struct timeval tmo;

    char buf[SIZE];

    //1. 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (-1 == sockfd)
    {
        perror("socket"); 
        goto err0;
    }

    //2. 初始化结构体
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "172.16.0.2", &addr.sin_addr.s_addr);


    //3. 连接服务器
    ret = connect(sockfd, (void*)&addr, sizeof(addr));
    if (-1 == ret)
    {
        perror("connect"); 
        goto err1;
    }
    printf("连接到服务端成功....\n");

    maxfd = sockfd + 1;

    //4. 发送或者接收数据
    while(1)
    {
        
        //清空集合
        FD_ZERO(&readfds);
    
        //添加需要监视的文件描述符
        FD_SET(STDIN_FILENO, &readfds); 
        FD_SET(sockfd, &readfds);

        //设置超时的时间
        tmo.tv_sec = 3; 
        tmo.tv_usec = 0;
    
       //readfds: 传入传出参数 
        ret = select(maxfd, &readfds, NULL, NULL, &tmo);
        if (-1 == ret)
        {
            perror("select"); 
            break;
        }
        else if (0 == ret)
        {
            printf("3'm timeout...\n"); 
        }
        else
        {
            //表示标准输入有数据读
            if (FD_ISSET(STDIN_FILENO, &readfds))    
            {
                memset(buf, 0, SIZE); 
                fgets(buf, SIZE, stdin);

                if (buf[strlen(buf) -1] == '\n')
                    buf[strlen(buf) - 1] = 0;
            
                //发送数据
                ret = send(sockfd, buf, strlen(buf), 0);
                if (-1 == ret)
                {
                    perror("send"); 
                    break;
                }
            }

            //表示套接字有数据可以读
            if (FD_ISSET(sockfd, &readfds))        
            {
                memset(buf, 0, SIZE); 

                //接收数据
                ret = recv(sockfd, buf, SIZE, 0);
                if (ret <= 0)
                {
                    perror("recv"); 
                    break;
                }

                printf("buf: %s\n", buf);
            }
        }
    }

    //5. 断开连接
    close(sockfd);

    return 0;
err1:
    close(sockfd);
err0:
    return 1;
}
