#include <linux/types.h>

#define getproclimit(pid, pl) syscall(342,pid,pl)
#define setproclimit(pid, limit, interval) syscall(342,pid,limit, interval)

struct proclimit {			// info and times about the process family of a given process
   pid_t root_pid;			// pid of the root process of the process family
   int time_limit;			// time limit for this process family
   int time_interval;			// time interval for enforcing the time limit
   unsigned long total_cpu_time; 	// total cpu time of the process family
   unsigned long max_cpu_time;  	// max cpu time of a process in this family
   unsigned long min_cpu_time;  	// min cpu time of a process in this family
};

