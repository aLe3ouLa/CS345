#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "proclimit.h"

#define INTERVAL 1000

void printProcLimit(struct proclimit * procLimitFields);

int main(int argc, char *argv[]){

    int pid,limit;
    struct proclimit *procLimitFields;

    procLimitFields = malloc(sizeof(struct proclimit));
    if(procLimitFields == NULL){
        printf("Not enough memory.\n");
        exit(1);
    }

    pid = atoi(argv[1]);
    limit = atoi(argv[2]);
    if(limit >= 1000){
        printf("limit must be less than 1000.\n");
        exit(0);
    }

    if (setproclimit(pid,limit,INTERVAL)!=0){
        perror("setproclimit");
        exit(1);
    }
    if(getproclimit(pid,procLimitFields)!=0){
        perror("getproclimit");
        exit(1);
    }        ;
    printProcLimit(procLimitFields);

    return 0;
}

void printProcLimit(struct proclimit * procLimitFields){

    printf("\nRoot pid: %d \nLimit: %d \nInterval: %d \nTotal cpu time: %lu \nMax cpu time: %lu \nMin cpu time: %lu",procLimitFields->root_pid
                                                ,procLimitFields->time_limit
                                                ,procLimitFields->time_interval
                                                ,procLimitFields->total_cpu_time
                                                ,procLimitFields->max_cpu_time
                                                ,procLimitFields->min_cpu_time);

}
