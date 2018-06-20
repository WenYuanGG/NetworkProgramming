#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

static int print_flag = 0;

static void sig_handler(int sig){
    //int retval;
    
    if ( sig == SIGCHLD ){ 
        //wait(&retval);
        //printf("CATCH SIGNAL PID = %d\n", getpid());
        //non-reentrant function (printf, scanf... stdio)
        print_flag = 1;
    }
}

int main(int argc, char *argv[]){
    pid_t pid; // process id
    pid_t child_pid;
    int exit_status;
    char *buffer = "";

    sig_t previous = signal(SIGCHLD, sig_handler);
    assert(previous == NULL);

    switch(pid = fork()){
        case -1:
            perror("fork");
            exit(-1);

        // Child process
        case 0:
            printf("[Child] PID is %d\n", getpid());
            printf("[Child] Parent's PID is %d\n", getppid());
            printf("[Child] Enter my exit status: ");

            // 以 fgets() + sscanf() 來取代 scanf()
            fgets(buffer, sizeof(buffer), stdin);
            sscanf(buffer, "%d", &exit_status);

            exit(exit_status);

        // Parent process
        default:
            printf("[PARENT] Child's PID is %d\n", pid);
            printf("[PARENT] I'm now waiting for my child to exit()...\n");
            printf("[PARENT] -----------------------------------\n\n");
            fflush(stdout); // what is fflush?
            child_pid = wait(&exit_status);

            // WEXITSTATUS is a macro
            printf("[PARENT] Child's exit status is [%d]\n", WEXITSTATUS(exit_status));

            if(print_flag == 1)
                printf("CATCH SIGNAL PID = %d\n", child_pid);
    }

    return 0;
}