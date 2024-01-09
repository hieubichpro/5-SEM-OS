#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ACTIVE_WRITER 0
#define WAITING_WRITERS 1
#define ACTIVE_READERS 2
#define WAITING_READERS 3

int flag = 1;

struct sembuf start_read[5] = {{WAITING_READERS, 1, 0}, 
                               {WAITING_WRITERS, 0, 0}, 
                               {ACTIVE_WRITER, 0, 0}, 
                               {WAITING_READERS, -1, 0}, 
                               {ACTIVE_READERS, 1, 0}};

struct sembuf stop_read[1] = {{ACTIVE_READERS, -1, 0}};

struct sembuf start_write[5] = {{WAITING_WRITERS, 1, 0}, 
                                {ACTIVE_WRITER, 0, 0}, 
                                {ACTIVE_READERS, 0, 0}, 
                                {WAITING_WRITERS, -1, 0}, 
                                {ACTIVE_WRITER, 1, 0}};

struct sembuf stop_write[1] = {{ACTIVE_WRITER, -1, 0}};

void writer(const int semfd, const int index, int *addr)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 10);
        if (semop(semfd, start_write, 5) == -1)
        {
            perror("Semop failed.\n");
            exit(1);
        }
        (*addr)++;
        printf("Писатель #%d (ID: %d) написал %d\n", index + 1, getpid(), *addr);
        if (semop(semfd, stop_write, 1) == -1)
        {
            perror("Semop failed.\n");
            exit(1);
        }
    }
    exit(0);
}

void reader(const int semfd, const int index, int *addr)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 10);
        if (semop(semfd, start_read, 5) == -1)
        {
            perror("Semop failed.\n");
            exit(1);
        }
        printf("Читатель #%d (ID: %d) прочитал %d\n", index + 1, getpid(), *addr);
        if (semop(semfd, stop_read, 1) == -1)
        {
            perror("Semop failed.\n");
            exit(1);
        }
    }
    exit(0);
}

void signal_handler(int signal)
{
    flag = 0;
    printf("Catch signal: %d, p_id = %d\n", signal, getpid());
}

int main()
{
    int semfd, shmfd;
    int ctl_ar, ctl_wr, ctl_aw, ctl_ww;
    int child_pid[5];
    int PERMS = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }
    if ((semfd = semget(IPC_PRIVATE, 4, IPC_CREAT | PERMS)) == -1)
    {
        perror("semget");
	    exit(1);
    }
    if (semctl(semfd, ACTIVE_READERS, SETVAL, 0) == -1)
    {
        perror("semctl");
        exit(1);
    }
    else if (semctl(semfd, WAITING_READERS, SETVAL, 0) == -1)
    {
        perror("semctl");
        exit(1);
    }
    else if (semctl(semfd, ACTIVE_WRITER, SETVAL, 0) == -1)
    {
        perror("semctl");
        exit(1);
    }
    else if (semctl(semfd, WAITING_WRITERS, SETVAL, 0) == -1)
    {
        perror("semctl");
        exit(1);
    }
    if ((shmfd = shmget(IPC_PRIVATE, 4, IPC_CREAT | PERMS)) == -1)
    {
        perror("shmget");
        exit(1);
    }
    int *addr = (int *)shmat(shmfd, 0, 0);
    if (addr == (int *) -1)
    {
        perror("shmat");
        exit(1);
    }
    (*addr) = 0;
    
    for (int i = 0; i < 3; i++)
    {
        child_pid[i] = fork();
        if (child_pid[i] == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        if (child_pid[i] == 0)
        {
            // while(1)
            writer(semfd, i, addr);
            // return 0;
        }
    }
    
    for (int i = 0; i < 5; i++)
    {
        child_pid[i] = fork();
        if (child_pid[i] == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        if (child_pid[i] == 0)
        {
            reader(semfd, i, addr);
            // return 0;
        }
    }
    
    int status;
    for (int i = 0; i < 8; i++)
    {
        pid_t childpid_r = wait(&status);
        if (WIFEXITED(status))
            printf("%d exit with status = %d.\n", childpid_r, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("%d exit from signal with status = %d.\n", childpid_r, WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("%d exit from signal with status = %d.\n", childpid_r, WSTOPSIG(status));
    }
    if (shmdt(addr) == -1)
    {
        perror("shmdt");
        exit(1);
    }
    
    return 0;
}
