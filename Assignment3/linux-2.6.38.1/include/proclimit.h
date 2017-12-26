#include <linux/types.h>

#define setproclimit(pid, limit, interval) syscall(341,pid,limit,interval)
#define getproclimit(pid,pl) syscall(342, pid, pl)

struct proclimit {			
   pid_t root_pid;			
   int time_limit;			
   int time_interval;			
   unsigned long total_cpu_time; 	
   unsigned long max_cpu_time;  	
   unsigned long min_cpu_time;  	
};
