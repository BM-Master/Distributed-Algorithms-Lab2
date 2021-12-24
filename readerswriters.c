#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

struct User
{
    int id;            // user ID
    int type;          // 0 - reader | 1 - writer
    int status;        // {0,1,2,3} -  {Reading, Writing, Waiting for Read, Waiting for Write}
    int db_operations; // Number of times read the DB
};

pthread_mutex_t mutex; // mutex for the DB access (readers-writers)

int setType()
{
    int ret = rand() % 2;
    return ret;
}

void *Computes(void *usr)
{
    struct User *user = (struct User *)usr; // thread destructuring
    if (user->type == 1)
    {
        printf("\n\n**************************************\n\n");
        printf("User %d %d < %d >\n ", user->id, user->db_operations, user->status);
        pthread_mutex_lock(&mutex);
        printf("From %d Beginning The Task OF WRITING IN THE DB\n\n", user->id);
        printf("From %d Ending The Task OF WRITING IN THE DB\n\n", user->id);
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        printf("\n\n**************************************\n\n");
        printf("User %d %d < %d >\n ", user->id, user->db_operations, user->status);
        printf("From %d Beginning The Task OF READING IN THE DB\n\n", user->id);
        printf("From %d Ending The Task OF READING IN THE DB\n\n", user->id);
    }
}

int main(int argc, char *argv[])
{
    pthread_t *thread;        // threads array
    pthread_attr_t attribute; // thread attributes

    struct User *auxUser;    // auxiliary user
    struct User **userArray; // array of users

    int totalUsers = atoi(argv[1]);             // total users
    int readerCapacity = atoi(argv[2]);         // reader capacity
    float writeTime = (atoi(argv[3])) / 1000.0; // writer time
    float readTime = writeTime * 0.25;          // reader time
    int userQuery = atoi(argv[4]);              // user query

    int i = 0;

    printf("n: %d \n", totalUsers);     // print total users
    printf("k: %d \n", readerCapacity); // print reader capacity
    printf("t: %f \n", writeTime);      // print writer time
    printf("i: %d \n", userQuery);      // print user query


    pthread_mutex_init(&mutex, NULL);                                 // initialize the mutex
    pthread_attr_init(&attribute);                                    // create a thread with default attributes
    thread = calloc(totalUsers, sizeof(pthread_t));        // allocate memory for the array of threads
    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE); // status finalization and thread id are preserved after the thread has finished
    userArray = calloc(totalUsers, sizeof(struct User *)); // allocate memory for the array of users

    for (i = 0; i < totalUsers; i++)
    {
        printf("Main: creating thread %d\n", i);
        auxUser = (struct User *)calloc(1, sizeof(struct User)); // allocate memory for user
        // setting parameters of each user
        auxUser->id = i;
        auxUser->type = setType();
        auxUser->status = auxUser->type == 1 ? 3 : 2;
        auxUser->db_operations = userQuery;

        userArray[i] = auxUser; // add user to the array of users
        pthread_create(&thread[i], &attribute, Computes, (void *)userArray[i]);
    }

    pthread_mutex_destroy(&mutex); // destroy the mutex
    return (0);
}
