#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>


pthread_mutex_t mutex;

typedef struct fdinfo
{
    int fd;
    int *maxfd;
    fd_set* rdset;
}FDInfo;

void* acceptConn(void* arg)
{
    printf("accept子线程线程ID: %ld\n", pthread_self());
    FDInfo* info = (FDInfo*)arg;

    int cfd = accept(info->fd, NULL, NULL);//这里读取数据fd做了拷贝，无需加锁
    
    pthread_mutex_lock(&mutex);
    FD_SET(cfd, info->rdset);
    *info->maxfd = cfd > *info->maxfd ? cfd : *info->maxfd;//这里info->maxfd要解引用
    pthread_mutex_unlock(&mutex);

    free(info);    //内存回收
    return NULL;
}

void* communication(void* arg)
{   
    printf("communication子线程线程ID: %ld\n", pthread_self());
    FDInfo* info = (FDInfo*)arg;
    char buf[1024];
    //int len = recv(info->fd, buf, sizeof(buf), 0);
    int len = recv(info->fd, buf, sizeof(buf), 0);
    if(len == -1)
    {
        perror("receive error");
        free(info);
        exit(0);
    }
    else if(len == 0)
    {
        printf("客户端已断开连接...\n");
        pthread_mutex_lock(&mutex);
        FD_CLR(info->fd, info->rdset);
        pthread_mutex_unlock(&mutex);
        close(info->fd);
        free(info);
        return NULL;   //客户端断开链接，直接退出子线程
        //break;
    }
    printf("read buf = %s\n", buf);
    //小写转大写
    for(int i=0;i<len;++i)
    {
        buf[i] = toupper(buf[i]);
    }
    printf("after buf = %s\n", buf);

    //大写发送回给客户端
    //write(info->fd, buf, strlen(buf)+1);
    int ret = send(info->fd, buf, strlen(buf)+1, 0);
    if(ret == -1)
    {
        perror("send error");
        exit(1);
    }

    free(info);
    return NULL;

}



//server
int main()
{
    pthread_mutex_init(&mutex, NULL);
    //创建监听的套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    //绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //绑定端口
    int ret = bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret == -1)
    {
        perror("bind error");
        exit(1);
    }

    //监听
    ret = listen(lfd, 128);
    if(ret == -1)
    {
        perror("listen error");
        exit(1);
    }

    fd_set redset;
    fd_set tmp;
    FD_ZERO(&redset);
    FD_SET(lfd, &redset);
    int maxfd = lfd;
    while(1)
    {   
        pthread_mutex_lock(&mutex);
        tmp = redset; //子线程要做写操作，所以要加锁
        pthread_mutex_unlock(&mutex);
        int ret = select(maxfd+1, &tmp, NULL, NULL, NULL);
        //判断是否是监听的fd
        if(FD_ISSET(lfd, &tmp))  //卧槽！！！
        {   
            //创建子线程
            pthread_t tid;
            FDInfo* info = (FDInfo*)malloc(sizeof(FDInfo));
            info->fd = lfd;
            info->maxfd = &maxfd;
            info->rdset = &redset;
            pthread_create(&tid,NULL,acceptConn,info);
            pthread_detach(tid);
        }
        for(int i=0;i<=maxfd;++i)
        {   
            if(i!=lfd && FD_ISSET(i, &tmp))
            {   
                pthread_t tid;
                FDInfo* info = (FDInfo*)malloc(sizeof(FDInfo));
                info->fd = i;
                info->rdset = &redset;
                pthread_create(&tid,NULL,communication,info);
                pthread_detach(tid);
            }
        }
    }

    close(lfd);
    pthread_mutex_destroy(&mutex);

    printf("bye!!\n");
    return 0;
}
