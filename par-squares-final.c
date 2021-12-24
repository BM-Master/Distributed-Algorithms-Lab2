/*
 * par-squares-final: computes the square roots for numbers between 1 and n using K threads
 *
 * Programmer: Ruben Carvajal Schiaffino
 *
 * Santiago de Chile: 13/11/2013
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


struct Message {

   int myid, nvalue, numthreads;
   float *data;
};   

/*
 *
 */
void *Computes(void *n) {

   int i, j, size;
   struct Message *m;
   float *s;

   printf("\n\n**************************************\n\n");
   fflush(stdout);
   m = (struct Message *) n; //desestructuración del thread
   printf("From %d Beginning The Task\n\n",m->myid);
   size = m->nvalue / m->numthreads;
   s = m->data;
   for (i = 0, j = m->myid * size + 1; i < size; i = i + 1, j = j + 1)
      s[i] = sqrt(j);
   for (i = 0; i < size; i++) {
      printf("Myid = %d %d.- %f\n",m->myid,i,m->data[i]);
      fflush(stdout);
   }
   printf("From %d Ending The Task\n\n",m->myid);   
   pthread_exit((void *) m);
}

/*
 *
 */
void *Usage(char *argv[]) {

   printf("Usage: %s n k\n", argv[0]);
   exit(1);
}


/*
 *
 */
int main(int argc, char *argv[]) {

   pthread_t *thread;
   pthread_attr_t attribute;
   struct Message **m;
   void *exit_status;
   int n, ii, j, k, l;
   float **s;

   if (argc != 3)
      Usage(argv);
   else {
      n = atoi(argv[1]);
      k = atoi(argv[2]);
      thread = calloc(k,sizeof(pthread_t));
      s = calloc(k,sizeof(float *));
      m = calloc(k,sizeof(struct Message *));
      for (ii = 0; ii < k; ii = ii + 1)
         s[ii] = calloc(n / k,sizeof(float));
      for (ii = 0; ii < k; ii = ii + 1)
         m[ii] = calloc(1,sizeof(struct Message));
      pthread_attr_init(&attribute); //crea un thread con los atributos por defecto
      pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_JOINABLE); // el estado de finalización y el id del thread se conservan después que el thread ha finalizado
      for (ii = 0; ii < k; ii = ii + 1) {
         printf("Main: creating thread %d\n", ii);
         m[ii]->myid = ii;
         m[ii]->nvalue = n;
         m[ii]->numthreads = k;
         m[ii]->data = s[ii];
         pthread_create(&thread[ii],&attribute,Computes,(void *) m[ii]);
      }
      pthread_attr_destroy(&attribute); 
      for (ii = 0; ii < k; ii = ii + 1) {
         pthread_join(thread[ii],&exit_status);
         m[ii] = (struct Message *) exit_status;
         printf("%d.- Message Received From %d\n",ii+1,m[ii]->myid);
         s[ii] = m[ii]->data;
      }
      printf("\n\n**************************************\n\n");
      for (j = 0, l = 1; j < k; j = j + 1)
         for (ii = 0; ii < n / k; ii = ii + 1, l = l + 1) 
            printf("%3d - %f\n",l,s[j][ii]);
   }
   return 0;
}
