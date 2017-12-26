#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "proclimit.h"

void printProcLimit(struct proclimit * procLimitFields);

int main(void){
    int i, count = 1;
    struct proclimit *procLimitFields;
    procLimitFields = malloc(sizeof(struct proclimit));
    if(procLimitFields == NULL){
        printf("Not enough memory.\n");
        exit(1);
    }

    if(setproclimit (-1, 10,11)!=0){
        perror("setproclimit");
        exit(1);
    }
    if(getproclimit(-1,procLimitFields)!=0){
        perror("getproclimit");
        exit(1);
    }
    printProcLimit(procLimitFields);

    for (i = 1; i < 1000001; i++){

        count = count*i;

    }

    if(getproclimit(-1,procLimitFields)!=0){
        perror("getproclimit");
        exit(1);
    }
    printProcLimit(procLimitFields);

    sleep(5);

    if(getproclimit(-1,procLimitFields)!=0){
        perror("getproclimit");
        exit(1);
    }
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
