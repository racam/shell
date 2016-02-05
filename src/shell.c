#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "readcmd.h"
#include "jobsList.h"

static void execute(struct cmdline *l);
static void executeCmd(char *const prog[], int const background, int const fdin, int const fdout);
static bool chooseAction(char *const prog[]);
static void signal_end_background_process(int sig);
static void shell_jobs();
static void shell_ulimit(char *const prog[]);


static struct cellule_t *jobsList = NULL;
static struct cellule_t *commandList = NULL;
static struct rlimit limitCpu;


//Let it go, let it go !
static void signal_end_background_process(int sig){
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
        //Get the job from the linked list
        struct cellule_t *prog = getJob(&jobsList, pid);
        if(prog != NULL){
            //Get the new time
            struct timeval t;
            if(gettimeofday(&t, NULL) == -1) perror("gettimeofday:");

            printf("child %d terminated in %d secondes\n", pid, (int)(t.tv_sec - prog->timer));
            deleteJob(&jobsList, pid);
        }
    }
}

int main() {
    limitCpu.rlim_cur = RLIM_INFINITY;
    limitCpu.rlim_max = RLIM_INFINITY;


    while (1) {
        struct cmdline *l;
        char *prompt = "shell>";

        l = readcmd(prompt);

        /* If input stream closed, normal termination */
        if (!l) {
            printf("exit\n");
            exit(EXIT_SUCCESS);
        }

        if (l->err) {
            /* Syntax error, read another command */
            printf("error: %s\n", l->err);
            continue;
        }

        /*Rewrite the signal for child end process*/
        struct sigaction action;
        action.sa_handler = signal_end_background_process;
        sigemptyset (&action.sa_mask);
        action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        if (sigaction(SIGCHLD, &action, NULL) == -1) perror("sigaction:");

        //Choose the right action
        execute(l);     

    }
}

//IT'S DUP TIME !
static void execute(struct cmdline *l){
    u_int i;
    int t_pipe[2];
    bool doSecondPipe = false;

    for(i=0; l->seq[i] != NULL; i++){
        int fdin = fileno(stdin), fdout = fileno(stdout);

        //Do the pipe IN (only the on the next program)
        if(doSecondPipe){
            if((fdin = dup(t_pipe[0])) == -1) perror("dup:1");
            doSecondPipe = false;
        }

        //We have a next command, we must pipe it if a pipe is not already in progress!
        if(l->seq[i+1] != NULL){
            //Init pipe into t_pipe
            if(pipe(t_pipe) == -1) perror("pipe:3");
            
            //Do the pipe OUT
            fdout = t_pipe[1];
            doSecondPipe = true;
        }

        //We have an input let's open it to read it !
        if(i==0 && l->in){
            if((fdin = open(l->in, O_RDONLY)) == -1) perror("open input:5");          
        }

        //We have an output let's open it to write it !
        if(l->seq[i+1] == NULL && l->out){
            fdout = open(l->out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if(fdout == -1) perror("open output:6");
        }

        //TRUE if bin FALSE if shell's command
        if(chooseAction(l->seq[i])){
            //Do the fork job (fork + exec + background)
            executeCmd(l->seq[i], l->bg, fdin, fdout);
        }

        if(fdin != 0)
            if(close(fdin) == -1) perror("close:7");
        if(fdout != 1)
            if(close(fdout) == -1) perror("close:8");
        
    }// end for
    
    //Wait for all the command to end
    struct cellule_t *cell = commandList;
    while(cell != NULL){
        int status;
        waitpid(cell->pid, &status, WUNTRACED | WCONTINUED);
        cell = cell->suiv;
    }
    deleteJobsList(&commandList);
    
}// end function


//IT'S FORK TIME
static void executeCmd(char *const prog[], int const background, int const fdin, int const fdout){
    pid_t pid;
    switch(pid = fork()){
        case -1: //Vader : Obi-Wan never told you what happened to your father.
            perror("fork:9");
            break;
        
        case 0: //Luke : He told me enough! He told me YOU killed him.    
            //Change the stdin & stdout
            if(dup2(fdin, fileno(stdin)) == -1) perror("dup2:10");
            if(dup2(fdout, fileno(stdout)) == -1) perror("dup2:11");

            //Add a CPU LIMIT (infiny by default)
            if(setrlimit(RLIMIT_CPU, &limitCpu) == -1) perror("setrlimit:");

            execvp(prog[0], prog);
            perror("execvp:");
            break;
        
        default: //Vader : No, I am your father. 
            {
                //Manage the timer
                struct timeval t;
                if(gettimeofday(&t, NULL) == -1) perror("gettimeofday:");
                
                //Manage background process
                if(background){
                    addJob(&jobsList, pid, prog[0], t.tv_sec);
                }else{
                    addJob(&commandList, pid, prog[0], t.tv_sec);
                }
            }
    }

}

//Look if it's a shell command if not execute the bin
static bool chooseAction(char *const prog[]){
    
    if(strcmp("jobs",prog[0]) == 0){
        shell_jobs();
        return false;
    }else if(strcmp("ulimit",prog[0]) == 0){
        shell_ulimit(prog);
        return false;
    }else{
        return true;
    }
}


//Print the background process who are still running
static void shell_jobs(){
    printJobsList(&jobsList);
}

//Set the ulimit (number of sec of CPU usage) for new process
static void shell_ulimit(char *const prog[]){
    if(prog[1] == NULL){
        printf("Usage : ulimit [seconds]\n");
    }else{
        int limit = atoi(prog[1]);

        if(limit <= 0){
            fprintf(stderr, "Invalid param, it must be > 0  : %s\n", prog[1]);
        }else{
            limitCpu.rlim_cur = limit;
            limitCpu.rlim_max = limit+5;
        }
    }
}
