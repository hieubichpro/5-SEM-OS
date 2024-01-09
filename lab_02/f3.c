#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void print_status(int status, pid_t childpid)
{
    if (WIFEXITED(status))                 // не равно нулю, если дочерний процесс успешно завершился.
        printf("Child with PID %d exited all right. Exit code: %d\n", childpid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))            // возвращает TRUE, если дочерний процесс завершился из-за необработанного сигнала.
        printf("Child with PID %d  exited with a non-intercepted signal number: %d\n", childpid, WTERMSIG(status));
    else if (WIFSTOPPED(status))                // возвращает TRUE, если дочерний процесс, из-за которого функция вернула управление, в настоящий момент остановлен;
        printf("Child with PID %d was stopped by signal: %d", childpid, WSTOPSIG(status));    // WSTOPSIG - возвращает номер сигнала, из-за которого дочерний процесс был остановлен.
    else
        printf("Process was terminated abnormally\n");
}

int main()
{
    char *exe[2] = {"./sum.out", "./square.out"};
    pid_t child_pid[2];
    int status;
    for (int i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(1);
        }
        else if (child_pid[i] == 0)
        {
            printf("child_%d: id %d ppid: %d pgrp: %d\n", i + 1, getpid(), getppid(), getpgrp());
	    	if (execl(exe[i], exe[i], NULL) == -1)
            {
	    	    perror("Error execl\n");
                exit(1);
            }
        }
        else
            printf("Parent: pid = %d, pgrp = %d, child = %d\n", getpid(), getpgrp(), child_pid[i]);
    }
    for (int i = 0; i < 2; i++)
    {
        if (waitpid(child_pid[i], &status, 0) == -1)
        {
            perror("Can't wait\n");
            exit(1);
        }
        else
        {
            printf("Child with PID %d finished\n", child_pid[i]);
            print_status(status, child_pid[i]);
        }
    }
    return 0;
}