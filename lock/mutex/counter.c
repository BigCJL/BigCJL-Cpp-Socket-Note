#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

#define MAX 50
//全局变量
int number;
pthread_mutex_t mutex;
//线程处理函数
void* funcA_num(void* arg){
    for(int i=0;i<MAX;i++){
        //pthread_mutex_lock(&mutex);
        int cur = number;
        cur++;
        usleep(10);  //放弃cpu时间片
        number = cur;
        printf("thread A=%lu,current number is: %d\n",pthread_self(),number);
        //pthread_mutex_unlock(&mutex);
    }
}

void* funcB_num(void* arg){
    for(int i=0;i<MAX;i++){
        //pthread_mutex_lock(&mutex);
        int cur = number;
        cur++;
        number = cur;
        printf("thread B=%lu,current number is: %d\n",pthread_self(),number);
        //pthread_mutex_unlock(&mutex);
        usleep(5);
    }
}
int main(int arg, const char* argv[])
{   pthread_t p1, p2;
    //创建两个子线程
    //pthread_mutex_init(&mutex, NULL);

    pthread_create(&p1, NULL, funcA_num, NULL);
    pthread_create(&p2, NULL, funcB_num, NULL);
    //阻塞，资源回收
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    //pthread_mutex_destroy(&mutex);

    return 0;
}

