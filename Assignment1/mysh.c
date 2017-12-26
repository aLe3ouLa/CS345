#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>


#define DELIMS " \t\r\n"
#define SEC 1000000
#define SIZE 1024

/** Global Variables are for typing in signals the PID and name of the command.*/
int globalPid;
char *name;


void type_prompt(char * ,char * );
void parseArguments(char *,char ** ,int ,int *, int *,int *);
void searchForRedirection (char ** ,int * ,int * ,int * ,int * ,int * ,int *);
void redirections (char ** ,char ** ,int ,int ,int ,int ,int ,int );
void printHistory (char ** ,char ** ,int ,int ,int ,int );
void printTime (double ,double ,double ,int );
void printArgs(char ** );
void signalIntHandler(int );
void signalTermHandler(int );
void searchForPipe(char ** , int *, int *);
void nextArg(char ** , char **,int * );
void spawn_proc (int ,int ,int ,char ** ,char ** ,char **);
void timeCounter(time_t , struct tms , struct tms , struct timeval , struct timeval , double * , double * , double * , long double );
int execvpe(const char *, char *const [],char *const []);

int main(int argc, char *argv[], char **envp)
{

    char * loginName, * directory,*tempargs[SIZE];
    char **args,  **commandHistory;
    char command[SIZE];
    int i, numArgs, status = 0, input, output, inputRedir = 0, outputRedir = 0,historyArg = 0,timeFlag;
    int ampershandFlag, pipeflag = 0, pipeCounter = 0, nextPipe = 0, appendFlag = 0, appendRedir = 0;
    pid_t pid;
    double realtime, cuser, csystem;
    struct tms start_tms;
    struct tms end_tms;
    long double clktck;
    struct timeval tv1,tv2;
    time_t starttime;

    /* Ignore the signals until we read a command*/
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    /*allocate arguments array */
    args = (char**) calloc(SIZE,sizeof(char *));
    for ( i = 0; i < SIZE; i++)
        args[i] = calloc(SIZE,sizeof(char));

    /* allocate history array */
    commandHistory = (char **) calloc (SIZE, sizeof(char *));
    for (i = 0; i < SIZE; i++)
        commandHistory[i] = calloc (SIZE, sizeof(char));

    /*allocate name*/
    loginName = malloc (SIZE* sizeof(char));

    if (loginName == NULL)
    {
        printf("Can't allocate memory");
        exit(0);
    }

    /*allocate directory*/
    directory = malloc (SIZE* sizeof(char));
    if (loginName == NULL)
    {
        printf("Can't allocate memory");
        exit(0);
    }

    /*
    Signals will act as signalHandlers functions define.
    SIGINT: type the signal and kill the process
    SIGTERM: type the signal and terminate the process

    The difference between this 2 is that SIGTERM  kill the process, gracefully or not,
    but to first allow it a chance to cleanup. But SIGINT just kills the process gracefully.

    */
    signal(SIGINT, signalIntHandler);
    signal(SIGTERM, signalTermHandler);

    /* shell starts */
    while (1){

        type_prompt(loginName, directory);

        input  = 0;
        output = 0;
        inputRedir = 0;
        outputRedir = 0;
        timeFlag = 0;
        ampershandFlag = 0;
        pipeflag = 0;
        pipeCounter = 0;
        appendFlag = 0;
        appendRedir = 0;

        if (!fgets(command, SIZE, stdin)) break; /* read the command*/

        if (strcmp(command, "\n"))
            strcpy(commandHistory[historyArg++%SIZE],command);

        parseArguments(command, args, SIZE, &numArgs , &timeFlag,&ampershandFlag); /* seperate the commands into strings*/

        /** TIME STARTS */
        clktck=sysconf(_SC_CLK_TCK);
        times(&start_tms);

        gettimeofday(&tv1, NULL);
        starttime=tv1.tv_sec;

        searchForRedirection (args, &input, & output, &inputRedir, &outputRedir, &appendFlag, &appendRedir);
        searchForPipe(args, &pipeflag, &pipeCounter);

        if (numArgs == 0){
            timeCounter( starttime, start_tms, end_tms,tv2, tv1, &realtime, &cuser, &csystem, clktck);
            printTime (cuser,csystem, realtime,timeFlag);
            continue; /* if there is no command, continue with the next loop */
        }

        name = args[0];

        if (strcmp (args[0],"exit") == 0) exit(0); /*Exit the shell*/


        if (strcmp (args[0], "cd") == 0){

            /*
            Cd will change the directory.
            In case of "cd  " will continue to the next loop.
            In case of error, we will get info for last mistake.

           */

            int status = chdir(args[1]);
            timeCounter( starttime, start_tms, end_tms,tv2, tv1, &realtime, &cuser, &csystem, clktck);
            printTime (cuser,csystem, realtime,timeFlag);

            if (numArgs == 1) continue;

            if (status != 0)
                perror("chdir");
            continue;
        }

        /* Fork: is an operation whereby a process creates a copy of itself.*/
        pid = fork();
        globalPid = pid;

        if (pid < 0){

            /* something wrong happened */
            perror("fork()");
            exit(1);
        }
        if (pid == 0){

            /* Child Process */
			int in,j;
			int fd[2];

            if (ampershandFlag){

                /* When we have ampershand we type pidd and name of process, and then exec the command without parent waits.*/
                printf("Pid: %d Name: %s \n", getpid(), args[0]);

            }

            if (input || output || appendFlag){

                /* Redirect the input or the output to a file and execute the command there*/
                redirections(tempargs, args, input, output, inputRedir , outputRedir, appendFlag, appendRedir );
                status = execvpe(*tempargs, tempargs, envp);
            }

             /* pipelines - we don't get into for if there are no pipes */
			in = 0;
			for (i  = 0; i < (pipeCounter); i++){
				pipe (fd);
				nextArg(args, tempargs ,&nextPipe);
				for ( j = 0; j < SIZE; j++)
				{
					if(tempargs[j] == NULL) break;
					fprintf(stderr, "%s ", tempargs[j]);
				}
				fprintf(stderr, "\n");

				spawn_proc (in, fd[1],status, tempargs, envp, commandHistory);
				close (fd [1]);

				/* Keep the read end of the pipe, the next child will read from there.  */
				in = fd [0];

			}
			if (in != 0)
				dup2 (in, 0);

			nextArg(args, tempargs ,&nextPipe);

			if (!strcmp (tempargs[0], "history")){
				printHistory(tempargs, commandHistory,output, outputRedir, appendFlag, appendRedir);
				exit(0);
			} else {
				status = execvpe(*tempargs, tempargs, envp);
				printf("%s : command not found", tempargs[0]);
				exit(0);
			}
        }

        if (pid != 0){

            /* parent process */
            int w;
            if (ampershandFlag) continue;

            w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
            if (w == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            timeCounter( starttime, start_tms, end_tms,tv2, tv1, &realtime, &cuser, &csystem, clktck);
            printTime (cuser,csystem, realtime,timeFlag);
        }

        printf("\n");
    }


    /* Free allocated space */
    free(loginName);

    free(directory);
    for (i = 0; i< SIZE; i++)
        free (args[i]);

    free(args);

    for (i = 0; i< SIZE; i++)
        free (commandHistory[i]);

    free(commandHistory);

    return 0;
}



void type_prompt(char * loginName, char * directory){
    /* types the prompt in the requested format */
    loginName = getlogin();
    getcwd(directory, SIZE);
    printf("%s@mysh:%s/ ", loginName, directory);

}

void parseArguments(char *buffer, char** args, int args_size, int*numArgs , int * timeFlag, int * ampershandFlag){

    /* seperate the arguments in the command that we read. We also check if we are in time or ampershand cases. */
    char *buf_args[args_size], *wbuf;
    char **cp;
    int  i = 0, j =0;


    wbuf=buffer;
    buf_args[0]=buffer;
    args[0] =buffer;

    cp=buf_args;
    while ((*cp=strsep(&wbuf,DELIMS)) != NULL ){
        if ((*cp != '\0') && (++cp >= &buf_args[args_size]))
            break;
    }

    for (j=0,i=0; buf_args[i]!=NULL; i++){
        if (!strcmp(buf_args[i], "mytime")){
            *timeFlag = 1;
            continue;
        }

        if (!strcmp(buf_args[i],"&")){
            *ampershandFlag = 1;
            continue;
        }

        if(strlen(buf_args[i])>0){
            args[j++]=buf_args[i];
        }

    }

    *numArgs = j;
    args[j] = NULL;
}

void searchForRedirection (char ** args, int * input, int * output, int * inputRedir, int * outputRedir, int * appendFlag, int * appendRedir){

    /* Search the arguments for a redirection. Make the flag 1 and store where the redirection is, if found one.*/

    int i = 0;
    for (i = 0; i< SIZE; i++){
        if (args[i] == NULL)
            break;

        if (!strcmp(args[i], "<")){
            *input = 1;
            *inputRedir = i;
        }

        if (!strcmp(args[i], ">")){
            *output = 1;
            *outputRedir = i;
        }
        if (!strcmp(args[i], ">>")){
            *appendFlag = 1;
            *appendRedir = i;
        }
    }
}

void searchForPipe(char ** args, int *pipeflag, int *pipeCounter){
    /* Check the arguments for pipeline. Works like the function of redirections.*/
    int i = 0;
    for (i = 0; i<SIZE; i++){
        if (args[i] == NULL)
            break;
        if (!strcmp(args[i], "|")){
            *pipeflag = 1;
            *pipeCounter = *pipeCounter + 1;
        }
    }

}
void redirections (char ** tempargs, char ** args, int  input, int  output, int inputRedir, int outputRedir, int appendFlag, int appendRedir){

    /*
    If we have redirection, we make the program write or read from the file.
    The >> appends to a file existed. Or Creates a new one if the file doesn't exist.
    The > writes in a file.
    The < reads from a file.

    */
    int i,j;

    if (input){
        int fd0 = open(args[inputRedir + 1], O_RDONLY, 0);
        dup2(fd0, STDIN_FILENO);
        close(fd0);
    }

    if (output){

        int fd1 = creat(args[outputRedir + 1], 0644);
        dup2(fd1, STDOUT_FILENO);
        close(fd1);
    }

    if (appendFlag){

        int fd2 = open(args[appendRedir + 1],O_WRONLY|O_APPEND);
        dup2(fd2, STDOUT_FILENO);
        close(fd2);

    }


    if (input || output || appendFlag){

        for(i = 0, j = 0; i < SIZE;){

            if(!strcmp(args[i],">") || !strcmp(args[i],"<") || !strcmp(args[i],">>")){
                i += 2; /*skip > and where you're supposed to redirect */
            }
            else
            {

                tempargs[j] = args[i];
                j++;
                i++;
            }

            if (args[i] == NULL) break;

        }
        tempargs[j] = NULL;
    }

    if (tempargs[0] == NULL){
        fprintf(stderr, "Not command to write in the %s", args[outputRedir + 1]);
        return;
    }

}

void printHistory (char ** args, char ** commandHistory, int output, int outputRedir, int appendFlag, int appendRedir){
    /*
    Prints the array of history. Checks if there is a redirection to write or append in file too.

    */

    int i;
    int saved_stdout = dup(STDOUT_FILENO);


    if (output)
    {
        int fd1 = creat(args[outputRedir + 1], 0644);
        dup2(fd1, STDOUT_FILENO);
        close(fd1);
    }

    if (output)
    {
        int start = outputRedir;
        args[start] = NULL;

    }

    if (appendFlag){

        int fd2 = open(args[appendRedir + 1],O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        dup2(fd2, STDOUT_FILENO);
        close(fd2);

    }

    for (i = 0; i < SIZE; i++){
        if (commandHistory[i][0] != '\0')
            printf("%d. %s\n", i+1, commandHistory[i]);
        else
            break;
    }


    if (output)
        dup2(saved_stdout, STDOUT_FILENO);

    if (appendFlag)
        dup2 (saved_stdout, STDOUT_FILENO);

}

void printTime (double cuser, double csystem, double realtime,int timeFlag){

    /* Print the time */

    if (timeFlag){
        printf("\nReal TIme: %lf\n",realtime);
        printf("User Time:%lf\nSystem Time:%lf\n",cuser,csystem);
    }

}

void printArgs(char ** args){
    int i;
    for (i = 0; i<SIZE; i++){
        if (args[i] == NULL) break;
        printf("args[%d]: %s", i,args[i]);
    }

}
void signalIntHandler(int signal){
    /* Handle the SIGNINT signals*/
    printf("\nSIGINT (%d) received\n", signal);
    printf("Pid: %d Name: %s\n", globalPid, name);

    if (globalPid!=0)
        kill(globalPid, signal);


}
void signalTermHandler(int signal){
   /* Handle the SIGTERM signals*/
    printf("\nSIGTERM (%d) received\n", signal);
    printf("Pid: %d Name: %s\n", globalPid, name);

    if (globalPid!=0)
        kill(globalPid, signal);

}

void nextArg(char ** args, char ** tempArgs ,int * nextPipe){
/* Find the next arg in a pipe */
    int i = 0;

    for (i = *nextPipe; i < SIZE; ){

        if (args[i] == NULL || !strcmp(args[i], "|")) break;
        if (args[i]!= NULL){
            while (args[i]!= NULL && strcmp(args[i], "|")){
                tempArgs[i-(*nextPipe)] = args[i];
                i++;

            }
            tempArgs[i-(*nextPipe)] = NULL;
            *nextPipe = i+1;
        }
    }

}

void spawn_proc (int in, int out, int status, char ** tempargs, char **envp, char **commandHistory){

    /* Implement the pipeline, and execute the commands*/

    pid_t childpid = fork();
    if (childpid < 0)
        perror ("fork");
    if (childpid == 0){
        if (in != 0){
            dup2 (in, 0);
            close (in);
        }

        if (out != 1){
            dup2 (out, 1);
            close (out);
        }

		if (!strcmp (tempargs[0], "history")){
			int i;

			for (i = 0; i < SIZE; i++){
				if (commandHistory[i][0] != '\0')
					printf("%d. %s\n", i+1, commandHistory[i]);
				else
					break;
			}
			exit(0);
		} else {
			status = execvpe(*tempargs, tempargs, envp);
			printf("%s : command not found", tempargs[0]);
			exit(0);
		}
    }
    if (childpid != 0){
        int w = waitpid(childpid, &status, WUNTRACED | WCONTINUED);
        if (w == -1){
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

    }
}

void timeCounter( time_t starttime, struct tms start_tms, struct tms end_tms, struct timeval tv2, struct timeval tv1, double * realtime, double * cuser, double * csystem, long double clktck){

    /* Stop counting, and calculate real, user and system time */
    time_t endtime;

    times(&end_tms);
    gettimeofday(&tv2, NULL);
    endtime=tv2.tv_sec;
    *realtime = (endtime-starttime) + (double)(tv2.tv_usec-tv1.tv_usec)/(double)SEC;
    *cuser=((double)end_tms.tms_cutime-(double)start_tms.tms_cutime)/(double)clktck;
    *csystem=((double)end_tms.tms_cstime-(double)start_tms.tms_cstime)/(double)clktck;
}
