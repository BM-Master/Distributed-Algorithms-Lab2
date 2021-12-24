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

// mutex definitions
pthread_mutex_t rw;
pthread_mutex_t r;
pthread_mutex_t rcap;

pthread_cond_t rcond; // condition variable for readers

int cnt = 1; 
int concurrentReaders; // number of concurrent readers
int dbop; // number of times the DB is accessed
int rc = 0; // number of readers accessing the DB

// set the type of the user
int setType()
{
    int ret = rand() % 2;
    return ret;
}

// set the status of the user
void callStatus(int status)
{
	switch (status)
	{
	case 0:
		printf("<Reading>\n");
		break;
	case 1:
		printf("<Writing>\n");
		break;
	case 2:
		printf("<Waiting for Read>\n");
		break;
	case 3:
		printf("<Waiting for Write>\n");
		break;
	default:
		printf("<Error>\n");
		break;
	}
}

// writer thread
void *writer(void *usrw)
{
    struct User *user = (struct User *)usrw; // thread destructuring
    while (user->db_operations > 0) // each user has to access db_operations times 
    {
        // print the user's status
        printf("User %d %d ", user->id, dbop - user->db_operations);
        callStatus(user->status);
     
        // printf("rc FROM WRITERS = %d\n", rc);
        // printf("WRITER ENTERS\n");
     
        pthread_mutex_lock(&rw); // lock the mutex for writing 
        cnt++; // increment the counter
    
        // printf("WRITING IN PROGRESS\n");

        user->status = 1; // set the status to writing

        // print the user's status
        printf("User %d %d ", user->id, dbop - user->db_operations);
        callStatus(user->status);
        // usleep(10000);

        printf("Writer %d modified cnt to %d\n", user->id, cnt); // print the cnt value
        pthread_mutex_unlock(&rw); // unlock the mutex for writing
        user->db_operations--; // decrement the number of times the user has access to the DB
        user->status = 3; // set the status to waiting for write
    }
    // return (NULL);
}

// reader thread
void *reader(void *usrr)
{
    struct User *user = (struct User *)usrr; // thread destructuring
    while (user->db_operations > 0) // each user has to access db_operations times 
    {
        // print the user's status
        printf("User %d %d ", user->id, dbop - user->db_operations);
        callStatus(user->status);
     
        // printf("rc FROM READERS = %d\n", rc);
        // printf("READER ENTERS\n");
     
        // check concurrent readers
        pthread_mutex_lock(&rcap); // lock the mutex for concurrent readers
        while (rc > concurrentReaders) // if the number of concurrent readers is greater than the max number of admitted readers
        {
            printf("TOO MUCH CONCURRENT READERS\n");
            pthread_cond_wait(&rcond, &rcap); // wait for readers to finish their operations
        }
        pthread_mutex_unlock(&rcap); // unlock the mutex for concurrent readers

        
        pthread_mutex_lock(&r); // Reader acquire the lock before modifying numreader

        if (rc == 0) 
        {
            pthread_mutex_lock(&rw); // reader block the writer 
            // printf("READER LOCKED rw\n");
        }
        rc++; // increment the number of readers (reader has started reading)
        pthread_mutex_unlock(&r); // Reader release the lock after modifying numreader

        // Reading Section
        // printf("READING IN PROGRESS\n");
        // printf("RC IN READING = %d\n", rc);

        user->status = 0; // set the status to reading

        // print the user's status
        printf("User %d %d ", user->id, dbop - user->db_operations);
        callStatus(user->status);

        // usleep(100000 * 0.25);

        printf("Reader %d: read cnt as %d\n", user->id, cnt); // print the cnt value as read 

        pthread_mutex_lock(&r); // Reader acquire the lock before modifying numreader
        rc--; // decrement the number of readers accessing the DB (reader has finished)
        pthread_cond_signal(&rcond); // signal to wake up waiting readers

        if (rc == 0) 
        { 
            pthread_mutex_unlock(&rw); // reader release the writer
        }
        pthread_mutex_unlock(&r); // Reader release the lock after modifying numreader
        user->db_operations--; // decrement the number of times the user has access to the DB
        user->status = 2; // set the status to waiting for read
    } 
    // return (NULL);
}

void main(int argc, char *argv[])
{
    srand(time(NULL));        // randomize seed
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
    pthread_mutex_init(&rcap, NULL);

    pthread_attr_init(&attribute);                                    // create a thread with default attributes
    threads = calloc(totalUsers, sizeof(pthread_t));                  // allocate memory for the array of threads
    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE); // status finalization and thread id are preserved after the thread has finished
    userArray = calloc(totalUsers, sizeof(struct User *));            // allocate memory for the array of users

    int totalReaders = 0;
    int totalWriters = 0;
    concurrentReaders = readerCapacity;
    dbop = userQuery;

    for (i = 0; i < totalUsers; i++)
    {
        // printf("Main: creating thread %d\n", i);
        auxUser = (struct User *)calloc(1, sizeof(struct User)); // allocate memory for user
        // setting parameters of each user
        auxUser->id = i;
        auxUser->type = setType();
        auxUser->type == 0 ? totalReaders++ : totalWriters++;
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
    pthread_mutex_destroy(&rcap);
    free(threads);
    free(userArray);
}