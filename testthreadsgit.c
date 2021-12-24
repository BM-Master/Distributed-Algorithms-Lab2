#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/*
This program provides a possible solution for first readers writers problem using mutex and semaphore.
I have used 10 readers and 5 producers to demonstrate the solution. You can always play with these values.
*/

pthread_mutex_t mutexResource;
pthread_mutex_t mutexWriters;
int cnt = 1;
int numreader = 0;

void *writer(void *wno)
{   
    pthread_mutex_lock(&mutexWriters);
    cnt = cnt*2;
    printf("WRITING IN PROGRESS");
    usleep(300000);
    printf("Writer %d modified cnt to %d\n",(*((int *)wno)),cnt);
    pthread_mutex_unlock(&mutexWriters);

}
void *reader(void *rno)
{   
    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutexResource);
    if (numreader++==0) pthread_mutex_lock(&mutexWriters);
    pthread_mutex_lock(&mutexResource);

    // Reading Section
    printf("Reader %d: read cnt as %d\n",*((int *)rno),cnt);
    usleep(200000);

    pthread_mutex_lock(&mutexResource);
    if (--numreader==0) pthread_mutex_unlock(&mutexWriters);
    pthread_mutex_unlock(&mutexResource);

    // numreader++;
    // if(numreader == 1) {
    //     sem_wait(&wrt); // If this id the first reader, then it will block the writer
    // }
    // pthread_mutex_unlock(&mutex);
    // // Reading Section
    // printf("Reader %d: read cnt as %d\n",*((int *)rno),cnt);

    // // Reader acquire the lock before modifying numreader
    // pthread_mutex_lock(&mutex);
    // numreader--;
    // if(numreader == 0) {
    //     sem_post(&wrt); // If this is the last reader, it will wake up the writer.
    // }
    // pthread_mutex_unlock(&mutex);
}

int main()
{   
    printf("STARTING");

    pthread_t read[10],write[5];
    pthread_mutex_init(&mutexResource, NULL);
    pthread_mutex_init(&mutexWriters, NULL);

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; //Just used for numbering the producer and consumer

    for(int i = 0; i < 10; i++) {
        pthread_create(&read[i], NULL, (void *)reader, (void *)&a[i]);
    }
    for(int i = 0; i < 5; i++) {
        pthread_create(&write[i], NULL, (void *)writer, (void *)&a[i]);
    }

    for(int i = 0; i < 10; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }

    pthread_mutex_destroy(&mutexResource);
    pthread_mutex_destroy(&mutexWriters);

    return 0;
    
}