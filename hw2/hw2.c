#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void sig_handler(int sig)
{
    int retval;
    if ( sig == SIGCHLD ){ 
        wait(&retval);
        printf("CATCH SIGNAL PID = %d\n",getpid());
        printf("%d\n", WEXITSTATUS(retval));
        //non-reentrant function (printf, scanf... stdio)
    }
}
 
int main(int argc, char *argv[]){
    pid_t pid;
    int exit_status;

    signal(SIGCHLD,sig_handler);

    switch(pid=fork()) {
        case -1:
            perror("fork");
            exit(-1);
 
        case 0: /*  Child Process */
            printf("[Child] PID is %d\n", getpid());
            printf("[Child] Parent's PID is %d\n", getppid());
            printf("[Child] Enter my exit status : ");
            scanf("%d", &exit_status);
            exit(exit_status);
 
        default:
            printf("[PARENT] Child's PID is %d\n", pid);
            printf("[PARENT] I'm now waiting for my child to exit()...\n");
            printf("[PARENT] ------------------------------------------\n\n");
            //fflush(stdout);
            wait(&exit_status);
            printf("[PARENT] Child's exit status is [%d]\n", WEXITSTATUS(exit_status));
    }

    return 0;
}