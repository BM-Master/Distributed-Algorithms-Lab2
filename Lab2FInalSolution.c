#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// user structure to control type of threads
struct User
{
    int id;            // user ID
    int type;          // 0 - reader | 1 - writer
    int status;        // {0,1,2,3} -  {Reading, Writing, Waiting for Read, Waiting for Write}
    int db_operations; // Number of times read the DB
};

pthread_mutex_t rw;
pthread_mutex_t r;
int cnt = 1;
int concurrentReaders;
int rc = 0;



int setType()
{
    int ret = rand() % 2;
    return ret;
}

void *writer(void *usrw)
{
    struct User *user = (struct User *)usrw; // thread destructuring
    // printf("rc FROM WRITERS = %d\n", rc);
    printf("WRITER ENTERS\n");
    pthread_mutex_lock(&rw);
    cnt++;
    // printf("WRITING IN PROGRESS\n");
    // usleep(300000);
    // printf("Writer %d modified cnt to %d\n", user->id, cnt);
    pthread_mutex_unlock(&rw);
}
void *reader(void *usrr)
{
    struct User *user = (struct User *)usrr; // thread destructuring
    // printf("rc FROM READERS = %d\n", rc);
    printf("READER ENTERS\n");
    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&r);
    if (rc == 0)
    {
        pthread_mutex_lock(&rw);
        printf("READER LOCKED rw\n");
    }
    rc++;
    pthread_mutex_unlock(&r);

    // Reading Section
    // printf("READING IN PROGRESS\n");
    // usleep(200000);
    // printf("Reader %d: read cnt as %d\n", user->id, cnt);
    printf("RC FROM READERS = %d\n", rc);

    pthread_mutex_lock(&r);
    rc--;
    if (rc == 0)
    {
        pthread_mutex_unlock(&rw);
    }
    pthread_mutex_unlock(&r);
}

void main(int argc, char *argv[])
{
    srand(time(NULL)); // randomize seed
    pthread_attr_t attribute; // thread attributes
    pthread_t *threads;

    struct User *auxUser;    // auxiliary user
    struct User **userArray; // array of users

    int totalUsers = atoi(argv[1]);             // total users
    int readerCapacity = atoi(argv[2]);         // reader capacity
    float writeTime = (atoi(argv[3])) / 1000.0; // writer time
    float readTime = writeTime * 0.25;          // reader time
    int userQuery = atoi(argv[4]);              // user query

    int i;

    printf("Total users: %d\n", totalUsers);
    printf("Reader capacity: %d\n", readerCapacity);
    printf("Writer time: %f\n", writeTime);
    printf("Reader time: %f\n", readTime);
    printf("User query: %d\n", userQuery);

    pthread_mutex_init(&r, NULL);
    pthread_mutex_init(&rw, NULL);

    pthread_attr_init(&attribute);                                    // create a thread with default attributes
    threads = calloc(totalUsers, sizeof(pthread_t));                  // allocate memory for the array of threads
    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE); // status finalization and thread id are preserved after the thread has finished
    userArray = calloc(totalUsers, sizeof(struct User *));            // allocate memory for the array of users

    int totalReaders = 0;
    int totalWriters = 0;
    concurrentReaders = readerCapacity;

    for (i = 0; i < totalUsers; i++)
    {
        // printf("Main: creating thread %d\n", i);
        auxUser = (struct User *)calloc(1, sizeof(struct User)); // allocate memory for user
        // setting parameters of each user
        auxUser->id = i;
        auxUser->type = setType();
        auxUser->type == 0 ? totalReaders++ : totalWriters++;
        printf("auxUser->type: %d\n", auxUser->type);
        auxUser->status = auxUser->type == 1 ? 3 : 2;
        auxUser->db_operations = userQuery;

        userArray[i] = auxUser; // add user to the array of users
                                // pthread_exit(NULL);
    }

    printf("TOTAL READERS: %d\n", totalReaders);
    printf("TOTAL WRITERS: %d\n", totalWriters);

    for (i = 0; i < totalUsers; i++)
    {
        if (userArray[i]->type == 0)
        {
            pthread_create(&threads[i], &attribute, reader, (void *)&userArray[i]->id);
        }
        else
        {
            pthread_create(&threads[i], &attribute, writer, (void *)&userArray[i]->id);
        }
    }

    for (i = 0; i < totalUsers; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("Main: all threads have terminated\n");
    pthread_attr_destroy(&attribute);
    pthread_mutex_destroy(&rw);
    pthread_mutex_destroy(&r);
    free(threads);
    free(userArray);
}