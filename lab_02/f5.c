#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <string.h>
#include <signal.h>

void print_status(int status);
int flag = 0;
void sig_handler(int sig_n)
{
    flag = 1;
	printf("\n СATCH %d\n", sig_n);
}

int main(void)
{
    signal(SIGINT, sig_handler);
    // sleep(2);    
    pause();
 	int fd[2];

	if (pipe(fd) == -1)
	{
	    perror("Pipe error\n");

	    exit(1);
	}
    
    pid_t children[2];
    int status;

	for (int i = 0; i < 2; i++)
	{
        if ((children[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(1);
        }
        else if (children[i] == 0)
        {
            char *str;
            if (i == 0)
                str = "bbbbbbbbbbbbbbbb\n";
            else 
                str = "aaaa\n";
            printf("Child %d, group %d, parent %d\n", getpid(), getpgrp(), getppid());
            if (flag)
            {
                close(fd[0]);
                write(fd[1], str, strlen(str));
                printf("%d sent message %s\n", getpid(), str);
            }
            else
            {
                printf("No signal.\n");
            }
           
	        exit(0);
	    }
    }
    for (int i = 0; i < 2; i++) 
    {
        children[i] = waitpid(children[i], &status, 0);

        printf("Child %d finished, status: %d\n", children[i], status);
        print_status(status);   
        if (flag)
        {
            printf("Receive \n");
            char c;
            do 
            {
                read(fd[0], &c, 1);
                printf("%c", c);
            }
            while (c != '\n');
        }           
    }
	
    
    return 0;
}

void print_status(int status)
{
    if (WIFEXITED(status))
        printf("Child finished, code: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Child terminated, signal %d\n", WTERMSIG(status));
    else if (WIFSTOPPED(status))
        printf("Child stopped, signal %d\n", WSTOPSIG(status));
}