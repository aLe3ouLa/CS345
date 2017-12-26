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
#include <linux/sched.h>
#include <linux/list.h>

static void setproclimit_dump(struct task_struct *cur, int root_pid, int limit, int interval){
    /*this function sets the limit and interval to process and its process family. */
	struct task_struct *childTask = NULL;
	struct list_head *children_list;
	cur->root_pid = root_pid; /* Make root of process family and change time_limit and time_interval. */
	cur->time_limit = limit;
	cur->time_interval = interval;
	printk("\nroot pid: %d, limit: %d, interval: %d\n",  cur->root_pid, cur->time_limit,cur->time_interval);
	
	/*if the progress has child change the fields too. */
	list_for_each(children_list, &cur->children) {
		childTask = list_entry(children_list,struct task_struct,sibling);
		setproclimit_dump(childTask,root_pid, limit, interval);

	}
}

asmlinkage int sys_setproclimit(int pid, int limit, int interval){

	struct task_struct *currentTask = NULL;
	struct task_struct *findProcess = NULL;

	printk("Name: Alexandra Barka AM: 2867 Function: setproclimit\n");

	/* if limit is greater than interval, then setproclimit must return error, with error value EINVAL. */
	if (limit > interval){
		return -EINVAL;
	}
	else{
		if (pid == -1){
			/* if pid  = -1, we want current process. */
			currentTask = get_current(); /*  get current process. */
			/* Change limit and interval of current process and its children. That, will make a process family.*/
			setproclimit_dump(currentTask, currentTask->pid,limit,interval);
		}else if (pid > -1 ){
			struct task_struct *task;
            /* if pid is greater than -1, we have to find the process. If the process doesn't exist then
                we return EINVAL.*/
			for_each_process(task) {
       			/* compare your pid with each of the task struct process pid*/
        			if (task->pid == pid) {
					/* if matched that is your user process PID */
              				findProcess = task;
					break;
				}
			}
			/* If the process didn't found. */
			if(findProcess == NULL){
				return -EINVAL;
			}
			/*if the process with this pid found, we change limit and interval to it AND its children.*/
			setproclimit_dump(findProcess, findProcess->pid,limit,interval);

		}
		else if (pid < -1) {
		    /* If the pid is negative we return the error value einval. */
			return -EINVAL;
		}
	}
    /* in case of success we return zero. */
	return((int)0);
}


