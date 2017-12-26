#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "proclimit.h"

void printProcLimit(struct proclimit *procLimitFields);

int main(void)
{

    struct proclimit *procLimitFields;
    pid_t pid;
    int status;

    procLimitFields = malloc(sizeof(struct proclimit));
    if(procLimitFields == NULL){
        printf("Not enough memory.\n");
        exit(1);
    }

    setproclimit (-1, 10,11);

    getproclimit(-1,procLimitFields);
    printProcLimit(procLimitFields);
    pid = fork();
    getproclimit(pid,procLimitFields);
    printProcLimit(procLimitFields);

    pid = fork();
    getproclimit(pid,procLimitFields);
    printProcLimit(procLimitFields);

    if(pid<0){
        perror("fork");
        exit(1);
    }
    if(pid == 0){
        sleep(5);
        getproclimit(pid,procLimitFields);
        printProcLimit(procLimitFields);
        pid = fork();
        printProcLimit(procLimitFields);
        getproclimit(pid,procLimitFields);
        printProcLimit(procLimitFields);
        if(pid == 0){
            sleep(5);
        }else{
            int wait = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        
        }
    }

    if(pid > 0){
        int wait;
        wait = waitpid(pid, &status, WUNTRACED | WCONTINUED);
    }

    return 0;
}

void printProcLimit(struct proclimit *procLimitFields){

printf("\nRoot pid: %d \nLimit: %d \nInterval: %d \nTotal cpu time: %lu \nMax cpu time: %lu \nMin cpu time: %lu",procLimitFields->root_pid
                                                ,procLimitFields->time_limit
                                                ,procLimitFields->time_interval
                                                ,procLimitFields->total_cpu_time
                                                ,procLimitFields->max_cpu_time
                                                ,procLimitFields->min_cpu_time);
}
