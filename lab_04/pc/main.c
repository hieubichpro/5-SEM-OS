#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
int flag = 1;
#define SB 0
#define SE 1
#define SF 2
#define P -1
#define V 1
struct sembuf PROD_LOCK[2] = { { SE, P, 0 }, { SB, P, 0 } };
struct sembuf PROD_RELEASE[2] = { { SB, V, 0 }, { SF, V, 0 } };
struct sembuf CONS_LOCK[2] = { { SF, P, 0 }, { SB, P, 0 } };
struct sembuf CONS_RELEASE[2] = { { SB, V, 0 }, { SE, V, 0 } };
void producer(const int sem_id, char **wpos, char *symb)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 3);
        if (semop(sem_id, PROD_LOCK, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        // char c = *symb;
        if (*symb >= 'z')
            *symb = 'a';
        **wpos = *symb;
        printf("Producer %d write: %c\n", getpid(), *symb);
        (*symb)++;
        (*wpos)++;

        if (semop(sem_id, PROD_RELEASE, 2) == -1)
        {
            perror("semop");
            exit(1);
        }
    }
    exit(0);
}
void consumer(const int sem_id, char **rpos)
{
    srand(time(NULL));
    while (flag)
    {
        sleep(rand() % 3);
        if (semop(sem_id, CONS_LOCK, 2) == -1)
        {
            perror("semop");
            exit(1);
        }
        // char c = **rpos;
        printf("Consumer %d read: %c\n", getpid(), **rpos);
        (*rpos)++;
        if (semop(sem_id, CONS_RELEASE, 2) == -1)
        {
            perror("semop");
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
    int childpid[3];
    int shmfd, semfd;
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    signal(SIGINT, signal_handler);
    if ((shmfd = shmget(IPC_PRIVATE, 128, IPC_CREAT | perms)) == -1)
    {
        perror("shmget");
        exit(1);
    }
    char *addr = (char *)shmat(shmfd, 0, 0);

    if (addr == (char *) -1) 
    {
        perror("shmat");
        exit(1);
    }
    
    char **rpos = (char **)addr;
    char **wpos = rpos + 1;
    char * symb = (char*)(wpos + 1);
    char *buf = symb + 1;
    *wpos = buf;
    *rpos = buf;
    *symb = 'a';
    if ((semfd = semget(IPC_PRIVATE, 3, IPC_CREAT | perms)) == -1)
    {
        perror("semget");
        exit(1);
    }

    if (semctl(semfd, SB, SETVAL, 1) == -1)
    {
        perror("semctl SB");
        exit(1);
    }
    else if(semctl(semfd, SF, SETVAL, 0) == -1)
    {
        perror("semctl SF");
        exit(1);       
    }
    else if(semctl(semfd, SE, SETVAL, 111) == -1)
    {
        perror("semctl SE");
        exit(1);
    }
    for (int i = 0; i < 3; i++)
    {
        childpid[i] = fork();
        if (childpid[i] == -1)
        {
            perror("Can't fork.\n");
            exit(1);
        }
        else if (childpid[i] == 0)
            producer(semfd, wpos, symb);
    }
    for (int j = 0; j < 3; j++)
    {
        childpid[j] = fork();
        if (childpid[j] == -1)
        {
            perror("Can't fork.\n");
            exit(1);
        }
        else if (childpid[j] == 0)
            consumer(semfd, rpos);
    }
    int status;
    for (int i = 0; i < 6; i++)
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
