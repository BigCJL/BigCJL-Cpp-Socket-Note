#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

#define MAX 50
//全局变量
int number;
pthread_rwlock_t rwlock;
//线程处理函数
void* read_num(void* arg){
    for(int i=0;i<MAX;i++){
        pthread_rwlock_rdlock(&rwlock);
        printf("thread read id=%lu,current number is: %d\n",pthread_self(),number);
        pthread_rwlock_unlock(&rwlock);
    }
}

void* write_num(void* arg){
    for(int i=0;i<MAX;i++){
        pthread_rwlock_wrlock(&rwlock);
        int cur = number;
        cur++;
        number = cur;
        printf("thread write id=%lu,current number is: %d\n",pthread_self(),number);
        pthread_rwlock_unlock(&rwlock);
        usleep(5);
    }
}
int main(int arg, const char* argv[])
{   pthread_t p1[5], p2[3];
    //创建两个子线程
    pthread_rwlock_init(&rwlock, NULL);
    for(int i=0;i<5;i++){
        pthread_create(&p1[i], NULL, read_num, NULL);
    }
    for(int i=0;i<3;i++){
        pthread_create(&p2[i], NULL, write_num, NULL);
    }

    for(int i=0;i<5;i++){
        pthread_join(p1[i], NULL);
    }
    for(int i=0;i<3;i++){
        pthread_join(p2[i], NULL);
    }
    //阻塞，资源回收
    pthread_rwlock_destroy(&rwlock);
 
    return 0;
}

