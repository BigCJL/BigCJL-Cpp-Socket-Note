#include <stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
//完成生产者和消费者模型
pthread_cond_t cond; //创建条件变量
pthread_mutex_t mutex;

struct Node
{
    int val;
    struct Node* next;
};
struct Node* head = NULL;
void* producer(void* arg){
    while(1){
        pthread_mutex_lock(&mutex);
        struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
        //初始化结点
        newNode->val = rand()%1000;
        newNode->next = head;
        head = newNode;
        //以上部分是头插法
        printf("生产者， id: %ld, number: %d\n", pthread_self(),newNode->val);
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond);  //生产之后 唤醒消费者
        sleep(rand()%3);
    }
    return NULL;
}

void* consumer(void* arg){
    while(1){
        pthread_mutex_lock(&mutex);
        while(head == NULL){
            //阻塞当前的消费者线程
            pthread_cond_wait(&cond, &mutex);
        }
        struct Node* node = head;
        printf("消费者, id: %ld, number: %d\n", pthread_self(),node->val);
        head = head->next;
        free(node);
        pthread_mutex_unlock(&mutex);

        sleep(rand()%3);
    }
    return NULL;    
}

int main()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    pthread_t t1[5], t2[5];
    for(int i=0;i<5;i++){
        pthread_create(&t1[i], NULL, producer, NULL);
    }

    for(int i=0;i<5;i++){
        pthread_create(&t2[i], NULL, consumer, NULL);
    }

    for(int i=0;i<5;i++){
        pthread_join(t1[i], NULL);
    }
    for(int i=0;i<5;i++){
        pthread_join(t2[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}

