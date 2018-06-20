#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    pid_t pid; // process id
    int exit_status;
    char *buffer = "";

    switch(pid = fork()){
        case -1:
            perror("fork");
            exit(-1);

        // Child process
        case 0:
            printf("[Child] PID is %d\n", getpid());
            printf("[Child] Parent's PID is %d\n", getppid());
            printf("[Child] Enter my exit status: ");
            
            // 以 fgets() + sscanf() 來取代 scanf , 比較安全
            fgets(buffer, sizeof(buffer), stdin);
            sscanf(buffer, "%d", &exit_status);
            
            exit(exit_status);

        // Parent process
        default:
            printf("[PARENT] Child's PID is %d\n", pid);
            printf("[PARENT] I'm now waiting for my child to exit()...\n");
            printf("[PARENT] -----------------------------------\n\n");
            fflush(stdout);
            wait(&exit_status);
            // WEXITSTATUS is a macro
            printf("[PARENT] Child's exit status is [%d]\n", WEXITSTATUS(exit_status));
    }

    return 0;
}