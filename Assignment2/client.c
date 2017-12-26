/**
Assignment 2 - client.c
Alexandra Barka
hy345 - Operating Systems
login: barka
AM: 2867
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>

#define PERMS 0600
#define SEC 1000000

typedef struct arguments
{
    int i;
    int M;
    int N;
    char *shm1;
    int *shm2;
    sem_t * shm3;
} args;

void *threadManipul(void *tid);

pthread_mutex_t mtx;
int thread_no = 0;

int main(int argc, char *argv[]){

    unsigned int s;
    struct sockaddr_un remote;
    int len ,i ,numofThreads = 0 ,N = 0 ,bytes_received = 0 ,status;
    int shmid[3]  = {0};
    key_t k[3] = {0};
    void *shm[3] = {NULL};
    pthread_t * threads;
    args temp;
    double time, realtime;
    struct timeval start, end;

    numofThreads = atoi(argv[1]); /* How many thread we work with.*/

    /** CREATE SOCKET */
    /* Call socket() to get a Unix domain socket to communicate through.*/
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    printf("Trying to connect...\n");

    /* Set up a struct sockaddr_un with the remote address (where the server is listening) and call connect() with that as an argument */
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, "guess_socket");
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1){
        perror("connect");
        exit(1);
    }

    printf("Connected.\n");

    /* Send how many threads we have to work with. */
    if (send(s, &numofThreads, sizeof(int),0) == -1){
        perror("send");
        exit(1);
    }
    /* Receive how many letters our client have to find. */
    bytes_received = recv(s, &N, sizeof(int),0);
    if (bytes_received < 0) perror("recv");

    /* Receive the keys to attach to shared memory. */
    bytes_received = recv(s, k, sizeof(k),0);
    if (bytes_received < 0) perror("recv");


    /** Attach client to shared memory */

    /* Attach and get a pointer to first memory segment. */
    if ((shmid[0] = shmget(k[0], N*sizeof(char), PERMS)) == -1)
    {
        perror("shmget");
        exit(1);
    }
    if((shm[0] = shmat(shmid[0], NULL,0)) == (char *)-1)
    {
        perror("shmat");
        exit(1);
    }
    /* Attach and get a pointer to second memory segment. */
    if ((shmid[1] = shmget(k[1], N *sizeof(int), PERMS)) == -1)
    {
        perror("shmget");
        exit(1);
    }

    if((shm[1] = shmat(shmid[1], NULL,0)) == (int *)-1)
    {
        perror("shmat");
        exit(1);
    }
    /* Attach and get a pointer to third memory segment. */
    if ((shmid[2] = shmget(k[2], 2*numofThreads*sizeof(sem_t),PERMS)) == -1)
    {
        perror("shmget");
        exit(1);
    }

    if((shm[2] = shmat(shmid[2], NULL,0)) == (sem_t *)-1)
    {
        perror("shmat");
        exit(1);
    }

    /* Initialize the structure we need to pass the arguments in threadManupil. */
    temp.i = thread_no;
    temp.N = N;
    temp.M = numofThreads;
    temp.shm1 = shm[0];
    temp.shm2 = shm[1];
    temp.shm3 = shm[2];

    threads = calloc (numofThreads, sizeof(pthread_t)); /* Allocate space for threads. */
    if(threads == NULL)
    {
        perror("calloc");
        exit(1);
    }
    gettimeofday(&start, NULL); /* Start counting how much time this process will take. */
    pthread_mutex_init(&mtx, NULL);

     /* Create threads. Call threadManipul for every thread. */
    for (i = 0; i < numofThreads; i++){

        status = pthread_create(&threads[i], NULL, threadManipul, (void *)&temp);
        if (status != 0){
            perror("pthread_create");
            exit(1);
        }
    }
     /* Wait until the thread i terminates. */
    for ( i = 0; i < numofThreads; i++)
        pthread_join(threads[i], NULL);

    printf("Client Guessed: %s\n", (char *)shm[0]);

    gettimeofday(&end, NULL);
    realtime = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec-start.tv_usec)/(double)SEC;
    printf("Counted Time (by client): %lf", realtime);

    /* close the connection with sever. */
    printf("\nConnection closed.\n");
    close(s);

    /* detach from the segment. */
    for(i = 0; i < 3; i++){
        if (shmdt(shm[i]) == -1){
            perror("shmdt");
            exit(1);
        }
    }

    /* Deallocates memory */
    free(threads);
    return 0;
}

void *threadManipul (void * t){

    /* Function threadManipul writes the guess of the client in first memory segment and
     reads from second memory segment if the guess was right or wrong. If it was right it stops guessing for
     that position. If it was wrong continues with next letter until it found it. */
    args tid = *(args *)t;
    int low, high,i,j;

    /* initialize the thread's variables */
    pthread_mutex_lock(&mtx);

    tid.i = thread_no++;
    low = tid.i * (tid.N /tid.M);
    high = (tid.i + 1) * (tid.N/ tid.M);

    pthread_mutex_unlock(&mtx);

    for (i = low; i < high; i++) /* N/M letters every semaphore controls.*/
        tid.shm1[i] = 'A';
    while(1){
        int k, flag = 0;
        sem_post(&tid.shm3[tid.i]); /* write the letter in first memory segment for server to pick up. */
        sem_wait(&tid.shm3[tid.i + tid.M]); /* wait until server reads the guess letter and respond if it's right or wrong.  */
        for(k = low; k < high; k++){
            if(tid.shm2[k] == 0){ /* if it's wrong guess the next till found it */
                tid.shm1[k]++;
                flag = 1;
            }
        }

        if(!flag) break;
    }
    pthread_exit(NULL);
}
