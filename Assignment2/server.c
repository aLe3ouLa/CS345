/**
Assignment 2 - server.c
Alexandra Barka
hy345 - Operating Systems
login: barka
AM: 2867
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>

#define PERMS 0600
#define SEC 1000000


typedef struct arguments{
    int i;
    int M;
    int N;
    char *shm1;
    int *shm2;
    sem_t * shm3;
} args;

void createString(int); /* Creates a random string */
void signalIntHandler(int); /* Handles ctrl c signal */
char *createFirstSharedMemory(key_t *,int ,int *); /* Creates 1st memory segment of chars. */
int *createSecondSharedMemory(key_t *,int ,int *); /* Creates 2nd memory segment of integers. */
sem_t *createThirdSharedMemory(key_t *,int ,int *,int **); /* Creates 3nd memory segment of semaphores. */
void *threadManipul (void *); /* Define what servers threads will do. */
void compare_with_string(char *,int * ,int ,int ); /* Compare the clients choice with string. */
int found(int *,int ,int ); /* Checks if the part of string that a thread handles found. */

char *secretString;
pthread_mutex_t mtx; /* Mutex used in threadManipul for initialization issues */
int thread_no = 0;
int socket_clean_up; /* for signals */

int main(int argc, char *argv[]){

    unsigned int s, s2;
    struct sockaddr_un local, remote;
    struct sockaddr *localptr, *remoteptr;
    int locallen ,remotelen ,N ,bytes_received ,M ,shmid1 = 0 ,shmid2 = 0 ,shmid3 = 0 ,*semid ,i ,status;
    double realtime;
    key_t k[3] = {0};
    void *shm[3] = {NULL};
    pthread_t *threads;
    args temp;
    struct timeval start, end;

    signal(SIGINT, SIG_IGN); /* Ignore signals until the socket is created. */

    srand(time(NULL));

    N = atoi(argv[1]); /* Get the string length. */
    if (N <= 0 ){
        perror("can't have negative or zero string");
        exit(0);
    }

    /** CREATE SOCKET */
    /*A call to socket() creates the Unix socket:*/
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    /** BIND SOCKET */

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, "guess_socket");
    unlink(local.sun_path);
    localptr = (struct sockaddr *) &local;
    locallen = sizeof(local);

    /* You got a socket descriptor from the call to socket(), bind that to an address in the Unix domain.*/
    if ((s2 = bind(s, localptr, locallen)) == -1) {
        perror("bind");
        exit(1);
    }
    socket_clean_up = s2;
    signal(SIGINT, signalIntHandler); /* After creating the socket we enable the signal handler. */

    /* listen for incoming connections from client programs. */
    if (listen(s, 1) == -1) {
        perror("listen");
        exit(1);
    }

    while (1){

        remoteptr =  (struct sockaddr *)  &remote;
        remotelen = sizeof(remote);
        printf("Waiting for a connection...\n");

        /**ACCEPT CLIENT */
        /* It extracts the first question request on the queue of pending connrctions & creates a new connected socket.
        It returns a new file descriptor reffering to that socket.*/

        if ((s2 = accept(s, remoteptr, (socklen_t *) &remotelen)) == -1) {
            perror("accept");
            exit(1);
        }

        printf("Connected.\n");
        printf("Waiting for Client to guess the secret String...\n\n");
        /*Create Secret String*/
        secretString = calloc (sizeof(char), N + 1);
        createString(N);
        /* Receive from socket how many threads will be created.*/
        bytes_received = recv(s2, &M, sizeof(int),0);
        if (bytes_received < 0) perror("recv");

        /*Array of semaphore id's*/
        semid = calloc (2*M, sizeof(int));
        if(semid == NULL){
            perror("calloc");
            exit(1);
        }

        /* Send how many characters has client to guess.*/
        if (send(s2, &N, sizeof(int),0) == -1){
            perror("send");
            exit(1);
        }
        /* Creates the three memory segments. */
        shm[0] = createFirstSharedMemory(&k[0], N, &shmid1);
        shm[1] = createSecondSharedMemory(&k[1], N, &shmid2);
        shm[2] = createThirdSharedMemory(&k[2], M, &shmid3, &semid);

        /* Send the keys to client to attach to memory*/
        if (send(s2,k, sizeof(k),0) == -1){
            perror("send");
            exit(1);
        }

        /* Initialize the structure with the data needed for threads.*/
        temp.i = thread_no;
        temp.N = N;
        temp.M = M;
        temp.shm1 = shm[0];
        temp.shm2 = shm[1];
        temp.shm3 = shm[2];

        /*Allocate thread array */
        threads = calloc (M, sizeof(pthread_t));
        if(threads == NULL){
            perror("calloc");
            exit(1);
        }

        gettimeofday(&start, NULL); /* Start calculating time.*/
        pthread_mutex_init(&mtx, NULL); /* Initialize mutex */

        /* Create threads. Call threadManipul for every thread. */
        for (i = 0; i < M; i++){
            status = pthread_create(&threads[i], NULL, threadManipul, (void *)&temp);
            if (status != 0){
                perror("pthread_create");
                exit(1);
            }
        }
        /* Wait until the thread i terminates. */
        for ( i = 0; i < M; i++)
            pthread_join(threads[i], NULL);

        gettimeofday(&end, NULL);
        realtime = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec-start.tv_usec)/(double)SEC; /* Calculates time */

        printf("Client found the secret string.\n");
        printf("Client found secret string in %lf sec\n", realtime);

        /* Close the connection with client. */
        close(s2);
        printf("Connection closed.\n");

        /* detach from the segment. */
        for(i = 0; i < 3; i++){
            if (shmdt(shm[i]) == -1){
                perror("shmdt");
                exit(1);
            }
        }
        /* Kill semaphores*/
        for (i = 0; i < 2*M; i++)
            semctl(semid[i], IPC_RMID, 0);

        /* Delete shared memory. */
        if (shmctl(shmid1, IPC_RMID, NULL) < 0){
            perror("shmctl shmid1");
            exit(1);
        }
        if (shmctl(shmid2, IPC_RMID, NULL) < 0){
            perror("shmctl shmid2");
            exit(1);
        }

        if (shmctl(shmid3, IPC_RMID, NULL) < 0){
            perror("shmctl shmid3");
            exit(1);
        }

        /* Deallocate the calloc'ed arrays*/
        free(threads);
        free(semid);
        free(secretString);

        thread_no = 0;

    }
    return 0;
}

void signalIntHandler(int signal){
    /* Handle the SIGNINT signals, close the socket connction and exits.*/
    printf("\nSIGINT (%d) received\n", signal);
    close(socket_clean_up);
    exit(0);
}

void createString(int N){
    /* Create a random string. */
    int i;
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (i = 0; i < N; i++)
        secretString[i]  = alphanum[rand() % (sizeof(alphanum) - 1)];

    secretString[N] = '\0';

}

char *createFirstSharedMemory(key_t *key, int N, int *shmid1){
    /*  Create the First shared Memory segment.
        First shared Memory segment is for N characters. Client writes in this segment its choices.
        Server initialize the N chars with '0'
    */

    int i;
    char* shm1;

    /* Create key */
    *key = ftok("./server.c", rand()%100);

    /* Allocate space for memory */
    *shmid1 = shmget(*key, N * sizeof(char), PERMS|IPC_CREAT);
    while (*shmid1 < 0){
        *key = ftok("./server.c", rand()%100);
        *shmid1 = shmget(*key, N * sizeof(char), PERMS|IPC_CREAT);
    }

    /* Attach memory*/
    if((shm1 = shmat(*shmid1, NULL,0)) == (char *)-1){
        perror("shmat");
        exit(1);
    }

    /* Initialize first segment of memory*/
    for (i = 0; i < N; i++)
        shm1[i] = '0';

    return shm1;
}

int *createSecondSharedMemory(key_t *key, int N, int *shmid2){
    /*  Create the  Second shared Memory segment.
        Second shared Memory segment is for N integers. Server writes in this segment 1 if character in the shm[i] is correct, else 0.
        Server initialize the N chars with 0
    */

    int i, *shm2;
    /* Create key*/
    *key = ftok("./server.c", rand()%100);

    /* Allocate space for memory */
    *shmid2 = shmget(*key, N * sizeof(int), PERMS|IPC_CREAT);

    while (*shmid2 < 0){
        *key = ftok("./server.c", rand()%100);
        *shmid2 = shmget(*key, N * sizeof(int), PERMS|IPC_CREAT);
    }
    /* Attach memory */
    if((shm2 = shmat(*shmid2, NULL,0)) == (int *)-1){
        perror("shmat");
        exit(1);
    }

    /*Initialize with zero */
    for (i = 0; i < N; i++)
        shm2[i] = 0;

    return shm2;
}

sem_t *createThirdSharedMemory(key_t *key, int M, int *shmid3, int ** semid){
 /*  Create the Third shared Memory segment.
        Third shared Memory segment is for 2*M semaphores.
        2 semaphores per thread. Initialize with sem_init.
    */
    int i;
    sem_t *shm1;
    /* Create key*/
    *key = ftok("./server.c", rand()%100);

     /* Allocate space for memory */
    *shmid3 = shmget(*key, 2*M*sizeof(sem_t), PERMS|IPC_CREAT|IPC_EXCL);
    while (*shmid3 < 0){
        *key = ftok("./server.c", rand()%100);
        *shmid3 = shmget(*key, 2*M*sizeof(sem_t), PERMS|IPC_CREAT|IPC_EXCL);
    }
    /* Attach memory */
    if((shm1 = shmat(*shmid3, NULL,0)) == (sem_t *)-1){
        perror("shmat");
        exit(1);
    }
    /* Initialize */
    for (i = 0; i < 2*M; i++)
        (*semid)[i] = sem_init(&shm1[i],1, 0);

    return shm1;

}

void *threadManipul (void * t){
    /* Server's threadManipul function read the clients' selection and compares it with secretString.
    Then, checks whether the choices are right or wrong and updates the second memory segment with 1 or 0, respectively. */

    args tid;
    int low, high;

    /* initialize the thread's variables */
    pthread_mutex_lock(&mtx);

    tid = *(args *)t;
    tid.i = thread_no++;
    low = tid.i * (tid.N /tid.M);
    high = (tid.i + 1) * (tid.N/ tid.M);

    pthread_mutex_unlock(&mtx);

    /* until all letters in the range the thread is responsible for are '1' */

    while(!found(tid.shm2, low, high)){
        sem_wait(&tid.shm3[tid.i]); /* wait until a client has written something in the shared memory */
        compare_with_string(tid.shm1, tid.shm2, low, high);  /* compare to the secret string and write the results to shared memory */
        sem_post(&tid.shm3[tid.i + tid.M]); /* make the second chunk available for the client's thread to pick up */
    }

    pthread_exit(NULL);
}

void compare_with_string(char * shm1, int *shm2, int low, int high){
/* Compares the secret  String and the guess of client*/
    int i;
    for (i = low; i < high; i++){
        if (shm1[i] == secretString[i])
            shm2[i] = 1;
        else
            shm2[i] = 0;
    }

}

int found (int *shm2, int low, int high){
    /* if the part of string that a threads controls has in second memory segment 1's return 1. else return 0;*/

    int i;
    for (i = low; i<high; i++)
        if(!shm2[i]) return 0;

    return 1;
}
