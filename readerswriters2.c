#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// #include <time.h>
// #include <windows.h>

// queue of concurrent readers
typedef struct
{
	int *buf;
	long head, tail;
	int full, empty;
	int capacity;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

// user structure to control type of threads
struct User
{
	int id;			   // user ID
	int type;		   // 0 - reader | 1 - writer
	int status;		   // {0,1,2,3} -  {Reading, Writing, Waiting for Read, Waiting for Write}
	int db_operations; // Number of times read the DB
};

queue *q;

queue *queueInit(int readerCap)
{
	queue *q;

	q = (queue *)malloc(sizeof(queue));
	if (q == NULL)
		return (NULL);

	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;
	q->capacity = readerCap;
	q->mut = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	q->buf = calloc(readerCap, sizeof(int));
	pthread_mutex_init(q->mut, NULL);
	q->notFull = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(q->notFull, NULL);
	q->notEmpty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(q->notEmpty, NULL);

	return (q);
}

void queueDelete(queue *q)
{
	pthread_mutex_destroy(q->mut);
	free(q->mut);
	pthread_cond_destroy(q->notFull);
	free(q->notFull);
	pthread_cond_destroy(q->notEmpty);
	free(q->notEmpty);
	free(q);
}

void queueAdd(queue *q, int in)
{
	q->buf[q->tail] = in;
	q->tail++;
	if (q->tail == q->capacity)
		q->tail = 0;
	if (q->tail == q->head)
		q->full = 1;
	q->empty = 0;

	return;
}

void queueDel(queue *q, int *out)
{
	*out = q->buf[q->head];

	q->head++;
	// q->buf[q->head] = 0;
	if (q->head == q->capacity)
		q->head = 0;
	if (q->head == q->tail)
		q->empty = 1;
	q->full = 0;

	return;
}

void queuePrint(queue *q)
{
	int i;
	printf("HEAD: %d\n", q->head);
	printf("TAIL: %d\n", q->tail);
	for (i = 0; i < q->capacity; i++)
		printf("%d ", q->buf[i]);
	printf("\n");
}

int setType()
{
	int ret = rand() % 2;
	return ret;
}

void callStatus(int status)
{
	switch (status)
	{
	case 0:
		printf("<Reading>\n");
		break;
	case 1:
		printf("<Writing>n");
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

void *Computes(void *usr)
{
	struct User *user = (struct User *)usr; // thread destructuring

	printf("User: %d %d", user->id, user->db_operations);
	callStatus(user->status);
	if (user->type == 1)
	{
		int k = 0;
		printf("\n\n**************************************\n\n");
		pthread_mutex_lock(q->mut);
		printf("WRITER INSIDE, NO ONE CAN DO OPERATIONS\n");
		printf("WRITING IN THE DATA BASE\n");
		usleep(3000000);
		// while(k < 100)
		// {
		// 	k++;
		// 	printf("%d\n", k);
		// }
		// printf("User TYPE: %d %d %d < %d >\n ", user->type, user->id, user->db_operations, user->status);
		printf("ENDING WRITER OPERATIONS\n");
		pthread_mutex_unlock(q->mut);
	}
	else
	{
		printf("User TYPE: %d %d %d < %d >\n ", user->type, user->id, user->db_operations, user->status);
		if (!q->full)
		{
			printf("INSIDE NOT FULL ADDING THREADS! %d \n", user->id);
			printf("DOING READING OPERATION! %d \n", user->id);
			queueAdd(q, user->id);
			usleep(1000000); // LO HIZO
		}
		while (q->full)
		{
			pthread_mutex_lock(q->mut);
			printf("QUEUE FULL!!!! HELP!!!!");
			// queueAdd(q, user->id);
			usleep(1000000);
			queueDel(q, &user->id);
			pthread_cond_wait(q->notFull, q->mut);
			pthread_mutex_unlock(q->mut);
		}
		pthread_cond_signal(q->notEmpty);
		queueDel(q, &user->id);

		printf("\n\n**************************************\n\n");
		printf("User TYPE: %d %d %d < %d >\n ", user->type, user->id, user->db_operations, user->status);
		printf("From %d Beginning The Task OF READING IN THE DB\n\n", user->id);
		printf("From %d Ending The Task OF READING IN THE DB\n\n", user->id);
	}
	return (NULL);
}

void main(int argc, char *argv[])
{
	pthread_attr_t attribute; // thread attributes
	pthread_t *threads;

	struct User *auxUser;	 // auxiliary user
	struct User **userArray; // array of users

	int totalUsers = atoi(argv[1]);				// total users
	int readerCapacity = atoi(argv[2]);			// reader capacity
	float writeTime = (atoi(argv[3])) / 1000.0; // writer time
	float readTime = writeTime * 0.25;			// reader time
	int userQuery = atoi(argv[4]);				// user query

	int i;

	printf("Total users: %d\n", totalUsers);
	printf("Reader capacity: %d\n", readerCapacity);
	printf("Writer time: %f\n", writeTime);
	printf("Reader time: %f\n", readTime);
	printf("User query: %d\n", userQuery);

	q = queueInit(readerCapacity);
	if (q == NULL)
	{
		fprintf(stderr, "main: Queue Init failed.\n");
		exit(1);
	}

	int a = 12;

	printf("EMPTY? %d\n", q->empty);
	queueAdd(q, 1);

	queuePrint(q);

	queueAdd(q, 2);

	queuePrint(q);

	queueAdd(q, 3);

	printf("FULL? %d\n", q->full);

	queuePrint(q);
	
	queueAdd(q, 4);

	queuePrint(q);

	queueAdd(q, 5);

	queuePrint(q);

	queueAdd(q, a);

	queuePrint(q);

	queueDel(q, &a);
	printf("FULL? %d\n", q->full);


	queueAdd(q, 20);

	queuePrint(q);
	printf("FULL? %d\n", q->full); 

	pthread_attr_init(&attribute);									  // create a thread with default attributes
	threads = calloc(totalUsers, sizeof(pthread_t));				  // allocate memory for the array of threads
	pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE); // status finalization and thread id are preserved after the thread has finished
	userArray = calloc(totalUsers, sizeof(struct User *));			  // allocate memory for the array of users

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
		pthread_create(&threads[i], &attribute, Computes, (void *)userArray[i]);
		// pthread_exit(NULL);
	}
	for (i = 0; i < totalUsers; i++)
	{
		pthread_join(threads[i], NULL);
	}
}
