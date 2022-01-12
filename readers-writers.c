/*
 *
 * Author: Bastian Martínez Soto
 *
 * Date: 23/12/2021
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// user structure to control type of threads
struct User
{
    int id; // user ID
    int type; // 0 - reader | 1 - writer
    int status; // {0,1,2,3} -  {Reading, Writing, Waiting for Read, Waiting for Write}
    int db_operations; // Number of times read the DB
};

// mutex definitions
pthread_mutex_t mwriter;
pthread_mutex_t mreader;
pthread_mutex_t rcap;

pthread_cond_t rcond; // condition variable for readers

int concurrentReaders; // number of concurrent readers
int dbop; // number of times the DB is accessed
int rc = 0; // number of readers accessing the DB
int wrTime; // time to write
int rdTime; // time to read

// set the type of the user
int setType()
{
    int ret = rand() % 2;
    return ret;
}

// set the status of the user
void callStatus(struct User* user)
{
	switch (user->status)
    {
	case 0:
        printf("User %d %d  <Reading>\n", user->id, dbop - user->db_operations);
	        break;
	case 1:
		        printf("User %d %d  <Writing>\n", user->id, dbop - user->db_operations);
		        break;
	case 2:
		        printf("User %d %d  <Waiting for Read>\n", user->id, dbop - user->db_operations);
		        break;
	case 3:
		        printf("User %d %d  <Waiting for Write>\n", user->id, dbop - user->db_operations);
		        break;
	default:
		        printf("<Error>\n");
		        break;
	}
}

// writer thread
void *writer(void *usrw)
{
    struct User *user;
    user = (struct User *)usrw; // thread destructuring
    while (user->db_operations > 0) // each user has to access db_operations times 
    {
        
        callStatus(user); // print the user's status
     
        pthread_mutex_lock(&mwriter); // lock the mutex for writing 

        user->status = 1; // set the status to writing

        callStatus(user); // print the user's status

        usleep(wrTime); 

        pthread_mutex_unlock(&mwriter); // unlock the mutex for writing
        // decrement the number of times the user has access to the DB
        user->db_operations--; 
        user->status = 3; // set the status to waiting for write
    }
}

// reader thread
void *reader(void *usrr)
{
    struct User *user;
    user = (struct User *)usrr; // thread destructuring
    while (user->db_operations > 0) // each user has to access db_operations times 
    {
        
        callStatus(user); // print the user's status
     
        // check concurrent readers
        pthread_mutex_lock(&rcap); // lock the mutex for concurrent readers
        // if the number of concurrent readers is greater than the number
        // of admitted readers
        while (rc > concurrentReaders) 
        {
            pthread_cond_wait(&rcond, &rcap); // wait for readers to finish
        }
        pthread_mutex_unlock(&rcap); // unlock the mutex for concurrent readers

        pthread_mutex_lock(&mreader); // Reader acquire the lock 
        rc++; // increment the number of readers (reader has started reading)
        pthread_mutex_unlock(&mreader);// Reader release the lock 
        if (rc == 1) 
        {
            pthread_mutex_lock(&mwriter); // reader block the writer 
        }

        // Reading Section
        user->status = 0; // set the status to reading

        callStatus(user); // print the user's status

        usleep(rdTime);

        pthread_mutex_lock(&mreader); // Reader acquire the lock 
        rc--; // decrement the number of readers accessing the DB (reader has finished)
        pthread_mutex_unlock(&mreader); // Reader release the lock 

        if (rc == 0) 
        { 
            pthread_mutex_unlock(&mwriter); // reader release the writer
        }
        pthread_cond_signal(&rcond); // signal to wake up waiting readers
        user->db_operations--; // decrement user operations left
        user->status = 2; // set the status to waiting for read
    } 
}

void main(int argc, char *argv[])
{
    srand(time(NULL)); // randomize seed
    pthread_attr_t attribute; // thread attributes
    pthread_t *threads;

    struct User *auxUser; // auxiliary user
    struct User **userArray; // array of users

    int totalUsers = atoi(argv[1]); // total users
    int readerCapacity = atoi(argv[2]); // reader capacity
    int writeTime = (atoi(argv[3])); // writer time
    int userQuery = atoi(argv[4]); // user query

    int i;

    pthread_mutex_init(&mreader, NULL);
    pthread_mutex_init(&mwriter, NULL);
    pthread_mutex_init(&rcap, NULL);

    pthread_attr_init(&attribute); 
    threads = calloc(totalUsers, sizeof(pthread_t)); 
    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE); 
    
    // allocate memory for the array of users
    userArray = calloc(totalUsers, sizeof(struct User *)); 

    concurrentReaders = readerCapacity;
    dbop = userQuery;
    wrTime = writeTime * 1000;
    rdTime = wrTime * 0.25;
    
    for (i = 0; i < totalUsers; i++)
    {
        auxUser = (struct User *)calloc(1, sizeof(struct User)); 
        
        // setting parameters of each user
        auxUser->id = i;
        auxUser->type = setType();
        auxUser->status = auxUser->type == 1 ? 3 : 2;
        auxUser->db_operations = userQuery;

        userArray[i] = auxUser; // add user to the array of users
    }

    for (i = 0; i < totalUsers; i++)
    {
        if (userArray[i]->type == 0)
        {
            pthread_create(&threads[i], &attribute, reader, (void *) userArray[i]);
        }
        else
        {
            pthread_create(&threads[i], &attribute, writer, (void *) userArray[i]);
        }
    }

    for (i = 0; i < totalUsers; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_attr_destroy(&attribute);
    pthread_mutex_destroy(&mwriter);
    pthread_mutex_destroy(&mreader);
    pthread_mutex_destroy(&rcap);
    
    free(threads);
    free(userArray);
}
