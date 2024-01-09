#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main()
{
    pid_t child_pid[2];
    for (int i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Can't fork :(\n");
            exit(1);
        }
        if (child_pid[i] == 0)
        {
            printf("child_%d: id %d ppid: %d pgrp: %d\n", i + 1, getpid(), getppid(), getpgrp());
            sleep(2);
            printf("child_%d: id %d ppid: %d pgrp: %d\n", i + 1, getpid(), getppid(), getpgrp());
            exit(0);
        }
        else
            printf("PARENT: id %d prgp: %d child_%d_pid: %d\n", getpid(), getpgrp(), i + 1, child_pid[i]);
    }
    return 0;
}
