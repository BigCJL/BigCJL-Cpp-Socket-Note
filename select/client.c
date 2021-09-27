#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    //1.创建用于通信的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(0);
    }

    //2.连接服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;  //ipv4
    addr.sin_port = htons(9999);
    inet_pton(AF_INET,"139.224.133.71",&addr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("socket");
        exit(0);
    }
    int i = 0;
    //3.communication
    while(1)
    {
        //读数据
        char recvBuf[1024];
        //写数据
        sprintf(recvBuf, "data: %d/n", i++);
        //fgets(recvBuf, sizeof(recvBuf), stdin);
        
        write(fd, recvBuf, strlen(recvBuf)+1);
        //如果客户端没有发送数据 默认阻塞
        read(fd, recvBuf, sizeof(recvBuf));
        printf("recv buf: %s\n", recvBuf);
        sleep(1);
    }

    close(fd);

    return 0;
}

