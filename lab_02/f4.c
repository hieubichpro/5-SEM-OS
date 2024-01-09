#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

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
    pid_t child_pid[2];
    int fd[2];
    int status;
    char *message[2] = {"aaa", "yyyyyyyyy"};
    char buf[30];
    if (pipe(fd) == -1)
    {
        perror("Can't pipe :(\n");
        exit(1);
    }
    for (int i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Can't fork :(\n");
            exit(1);
        }
        if (child_pid[i] == 0)      // Потомок читает из канала
        {
            close(fd[0]);           // Закрывает неиспользуемый конец для записи
            write(fd[1], message[i], sizeof(message[i]));
            printf("msg from child (pid = %d) %s was sent\n", getpid(), message[i]);
            exit(0);
        }
        else
        {
            for (int i = 0 ; i < sizeof(buf);i++)
                buf[i] = 0;
            printf("parent pid: %d, child %d, group %d\n", getpid(), child_pid[i], getpgrp());
            waitpid(child_pid[i], &status, 0);
            print_status(status, child_pid[i]);
            // close(fd[1]);               // Закрывает неиспользуемый конец для чтения
            read(fd[0], buf, sizeof(buf));
            printf("Parent read message %s\n\n", buf);
        }
}
    // buf[1] = '\0';
    for (int i = 0 ; i < sizeof(buf);i++)
    buf[i] = 0;
    close(fd[1]);               
    read(fd[0], buf, sizeof(buf));
    printf("3-rd. Parent read message %s\n\n", buf);
    return 0;
}
