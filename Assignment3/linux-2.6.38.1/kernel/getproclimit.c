/*
* Name: Alexandra Barka
* login: barka@csd.uoc.gr
* am: 2867
* hy345 - Operating Systems
* Exercise_3 - Add two new system calls to linux kernel.
*/

#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <asm-generic/errno-base.h>
#include <asm/cputime.h>
#include <proclimit.h>


asmlinkage int sys_getproclimit(int pid, struct proclimit *pl){
    struct task_struct *currentTask = NULL;
    struct task_struct *findProcess = NULL;
    unsigned long sum = 0, min, max;

    printk("Name: Alexandra Barka AM: 2867 Function: getproclimit\n");

    if (pl == NULL){
        /*if pointer to struct we want to fill with information is NULL, return EINVAL error. */
        return -EINVAL;
    }
    if (pid == -1){
        struct proclimit temp;

	/*Checks the validity of the user space memory pointer*/
        if (!access_ok(VERIFY_WRITE, pl, sizeof(struct proclimit))) return -EFAULT;
        
	/* copy from user-level program the struct we want to fill with information. */
        if (copy_from_user(&temp, pl, sizeof(struct proclimit)) != 0){
            return -EINVAL; /* in case of error*/
        }
        
        currentTask = get_current(); /*  get current process. */
        /*Copy to struct ifo for the process*/
        temp.root_pid = currentTask->root_pid;
        temp.time_limit = currentTask->time_limit;
        temp.time_interval = currentTask->time_interval;
        /* at start min & max has the total time of the first process*/
        min = cputime_to_usecs(currentTask->stime) + cputime_to_usecs(currentTask->utime);
        max = cputime_to_usecs(currentTask->stime) + cputime_to_usecs(currentTask->utime);
        if (temp.root_pid == -1){
            /* if the process don't belong in process family*/
            temp.total_cpu_time = -1;
            temp.max_cpu_time = -1;
            temp.min_cpu_time = -1;
        }
        else{
            struct task_struct *task;
            cputime_t i;
            /*calculation of total cpu time of process family*/
            for_each_process(task){
                if (task->root_pid == temp.root_pid){
                    i = cputime_to_usecs(task->stime) + cputime_to_usecs(task->utime);
                    if (min < i){
                        min = i; /*if exists a process with less cpu time, make it min*/
                    }
                    if (max > i){
                        max = i;/*if exists a process with more cpu time, make her it max*/
                    }
                    sum = sum + i; /*calculate total*/
                }
            }
            /*copy the values to struct*/
            temp.total_cpu_time = sum;
            temp.max_cpu_time = max;
            temp.min_cpu_time = min;
        }

        /*copy the information for the process in user space*/
        if (copy_to_user(pl, &temp, sizeof(struct proclimit))!=0){
            return -EINVAL;
        }
        printk("root: %d limit: %d interval: %d total: %lu min: %lu max: %lu", temp.root_pid,temp.time_limit,temp.time_interval, temp.total_cpu_time,temp.max_cpu_time,temp.min_cpu_time);

    }
    else if (pid > -1 ){
        /*if the process is not current, we first find the process and then do similar steps with the current*/
        struct task_struct *task;
        struct proclimit temp;

        /*find process*/
        for_each_process(task){

            if (task->pid == pid){
                findProcess = task;
                break;
            }
        }
        /*if the process doesnt exist return error*/
        if(findProcess == NULL){
            return -EINVAL;
        }
	
	/*Checks the validity of the user space memory pointer*/
        if (!access_ok(VERIFY_WRITE, pl, sizeof(struct proclimit))) return -EFAULT;

        /* copy from user-level program the struct we want to fill with information. */
        if (copy_from_user(&temp, pl, sizeof(struct proclimit)) != 0){
            return -EINVAL;
        }
         
        /*Copy to struct ifo for the process*/
        temp.root_pid = findProcess->root_pid;
        temp.time_limit = findProcess->time_limit;
        temp.time_interval = findProcess->time_interval;

        /* at start min & max has the total time of the first process*/
        min = cputime_to_usecs(findProcess->stime) + cputime_to_usecs(findProcess->utime);
        max = cputime_to_usecs(findProcess->stime) + cputime_to_usecs(findProcess->utime);
        if (temp.root_pid == -1){
            /* if the process don't belong in process family*/
            temp.total_cpu_time = -1;
            temp.max_cpu_time = -1;
            temp.min_cpu_time = -1;
        }
        else{
            struct task_struct *task;
            cputime_t i;
            /*calculation of total cpu time of process family*/
            for_each_process(task){
                if (task->root_pid == temp.root_pid){
                    i = cputime_to_usecs(task->stime) + cputime_to_usecs(task->utime);
                    if (min < i){
                        min = i;/*if exists a process with less cpu time, make it min*/
                    }
                    if (max > i){
                        max = i;/*if exists a process with more cpu time, make it max*/
                    }
                    sum = sum + i; /*calculate total*/
                }
            }
            /*copy the values to struct*/
            temp.total_cpu_time = sum;
            temp.max_cpu_time = max;
            temp.min_cpu_time = min;

        }
        /*copy the information for the process in user space*/
        if (copy_to_user(pl, &temp, sizeof(struct proclimit))!=0){
            return -EINVAL;
        }

    printk("root: %d limit: %d interval: %d total: %lu min: %lu max: %lu", temp.root_pid,temp.time_limit,temp.time_interval, temp.total_cpu_time,temp.max_cpu_time,temp.min_cpu_time);
    }
    else if (pid < -1){
        /*if the pid is negative*/
        return -EINVAL;
    }
    return((int)0);
}


