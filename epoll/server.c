#include <stdio.h>
#include <sys/epoll.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, const char* argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket!");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int optval = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    //设置端口复用？？
    
    bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    int ret = listen(lfd, 64);
    if(ret == -1)
    {
        perror("listem");
        exit(1);
    }

    //创建epoll实例
    int epfd = epoll_create(1);
    if(epfd == -1)
    {
        perror("epoll create");
        exit(0);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    struct epoll_event evs[1024];
    int size = sizeof(evs)/sizeof(evs[0]);
    //检测
    while(1)
    {
        int num =  epoll_wait(epfd, evs, size, -1);//就绪的文件描述符的总个数
        printf("num = %d\n", num);
        for(int i=0;i<num;i++)
        {
            struct sockaddr_in caddr;
            int addrlen = sizeof(caddr);
            int fd = evs[i].data.fd;    //读出
            if(fd == lfd)
            {   
                int cfd = accept(fd, (struct sockaddr*)&caddr, (socklen_t*)&addrlen);  //不需要传出 直接写NULL
                struct epoll_event ev;  //这一句可以不要
                ev.events = EPOLLIN;
                ev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);  //ev在传递后 会做一个拷贝
            }
            else
            {
                char buf[1024];
                int len = recv(fd, buf, sizeof(buf), 0);
                if(len == -1)
                {
                    perror("recv error");
                    exit(1);
                }
                else if(len == 0)
                {   
                    char ip[32];
                    printf("客户端:%s已断开连接。。。\n", inet_ntop(AF_INET, &caddr.sin_addr.s_addr,ip,sizeof(ip)));
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);  //删除 不需要指定事件
                    close(fd);
                    break;
                }
                printf("read buf = :%s\n", buf);
                for(int i=0; i<len; ++i)
                {
                    buf[i] = toupper(buf[i]);
                }
                printf("after buf = :%s\n", buf);

                ret = send(fd, buf, strlen(buf)+1, 0);
                if(ret == -1)
                {
                    perror("send");
                    exit(0);
                }
            }
        }
    }
    close(lfd);
    return 0;
}


