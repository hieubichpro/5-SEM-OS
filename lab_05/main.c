#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include <signal.h>

HANDLE mutex;
HANDLE can_read;
HANDLE can_write;

long waiting_writers = 0;
long waiting_readers = 0;
long active_readers = 0;
bool active_writer = false;

int counter = 0;

int flag = 1;
void sig_handler(int signal)
{
    flag = 0;
    printf("CATCH %d \n", signal);
}

void start_read(void)
{
    InterlockedIncrement(&waiting_readers);

    if (active_writer || waiting_writers > 0)
        WaitForSingleObject(can_read, INFINITE);

    WaitForSingleObject(mutex, INFINITE);

    InterlockedDecrement(&waiting_readers);
    SetEvent(can_read);
    InterlockedIncrement(&active_readers);

    ReleaseMutex(mutex);
}

void stop_read(void)
{
    InterlockedDecrement(&active_readers);

    if (active_readers == 0)
    {
       ResetEvent(can_read);
       SetEvent(can_write);
    }
}

void start_write(void)
{
    InterlockedIncrement(&waiting_writers);

    if (active_writer || active_readers > 0)
        WaitForSingleObject(can_write, INFINITE);

    InterlockedDecrement(&waiting_writers);
}

void stop_write(void)
{
    ResetEvent(can_write);
    if (waiting_readers > 0)
        SetEvent(can_read);
    else
        SetEvent(can_write);
}

DWORD run_reader(CONST LPVOID param)
{
    int id = *(int *)param;
    srand(time(NULL));
    while (flag)
    {
        Sleep(rand() % 2000 + 5);
        start_read();
        printf("Reader #%d ID: %d read %d\n", id, GetCurrentThreadId(), counter);
        stop_read();
    }
    return 0;
}

DWORD run_writer(CONST LPVOID param)
{
    int id = *(int *)param;
    srand(time(NULL));
    while (flag)
    {
        Sleep(rand() % 2000);
        start_write();
        counter++;
        printf("Writer #%d ID: %d write %d\n", id, GetCurrentThreadId(), counter);
        stop_write();
    }
    return 0;
}

int main()
{
    setbuf(stdout, NULL);
    DWORD thid[8];
    HANDLE pthread[8];
    int rw_id[8];
    void *handler = NULL;
    if ((handler = signal(SIGINT, sig_handler)) == SIG_ERR)
    {
        perror("signal.\n");
        ExitProcess(-1);
    }

    if ((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
        perror("CreateMutex");
        return -1;
    }

    if ((can_read = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        perror("CreateEVent");
        return -1;
    }

    if ((can_write = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        perror("CreateEVent");
        return -1;
    }

    for (size_t i = 0; i < 5; i++)
    {
        rw_id[i] = i + 1;
        pthread[i] = CreateThread(NULL, 0, run_reader, rw_id + i, 0, &thid[i]);
        if (pthread[i] == NULL)
        {
            perror("CreateThread");
            ExitProcess(-1);
        }
    }

    for (size_t i = 5; i < 8; i++)
    {
        rw_id[i] = i - 4;
        pthread[i] = CreateThread(NULL, 0, run_writer, rw_id + i, 0, &thid[i]);

        if (pthread[i] == NULL)
        {
            perror("CreateThread");
            ExitProcess(-1);
        }
    }

    for (int i = 0; i < 8; i++)
    {
        DWORD dw = WaitForSingleObject(pthread[i], INFINITE);
        switch (dw)
        {
        case WAIT_OBJECT_0:
            printf("thread %d finished successfully\n", thid[i]);
            break;
        case WAIT_TIMEOUT:
            printf("waitThread timeout %d\n", dw);
            break;
        case WAIT_FAILED:
            printf("waitThread failed %d\n", dw);
            break;
        }
    }

    CloseHandle(mutex);
    CloseHandle(can_read);
    CloseHandle(can_write);

    return 0;
}