#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "bakery.h"

#define MAX_CLIENTS_COUNT 128

static char counter = 'a';
static int cur = 0;

static bool choosing[MAX_CLIENTS_COUNT] = {false};
static int numbers[MAX_CLIENTS_COUNT] = {0};
static int pids[MAX_CLIENTS_COUNT] = {0};

int max_number()
{
	int res = numbers[0];

	for (int i = 1; i < MAX_CLIENTS_COUNT; i++)
		if (numbers[i] > res)
			res = numbers[i];

	return res;
}

void *
get_number(void *arg)
{
	static struct REQUEST  result;
	struct REQUEST *argp = arg;
	argp->index = result.index = cur;
	cur++;

	pids[argp->index] = argp->pid;

	choosing[argp->index] = true;
	int next = max_number() + 1;
	result.number = next;
	numbers[argp->index] = next;
	choosing[argp->index] = false;

	printf("Client (pid %d) receive number %d\n", argp->pid, result.number);

	return &result;
}


struct REQUEST *
get_number_1_svc(struct REQUEST *argp, struct svc_req *rqstp)
{
	static struct REQUEST  result;
	void *tmp;
	int s;
	pthread_t thr;
	pthread_attr_t attr;
	s = pthread_attr_init(&attr);
	if (s != 0)
	{
		perror("pthread_attr_init");
		exit(1);
	}
	s = pthread_create(&thr, &attr, get_number, argp);
	if (s != 0)
	{
		perror("pthread_create");
		exit(1);
	}
	s = pthread_attr_destroy(&attr);
	if (s != 0)
	{
		perror("pthread_attr_destroy");
		exit(1);
	}
	s = pthread_join(thr, &tmp);
	if (s != 0)
	{
		perror("pthread_join");
		exit(1);
	}
	result = *(struct REQUEST *)tmp;
	
	return &result;
}


void *
bakery_service(void *arg)
{
	static int  result;
	struct REQUEST *argp = arg;
	
	for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
	{
		while (choosing[i]) {}
		while (numbers[i] != 0 && (numbers[i] < numbers[argp->index] || 
								   numbers[i] == numbers[argp->index] && 
								   pids[i] < pids[argp->index])) {}
	}

	result = counter++;
	numbers[argp->index] = 0;

	printf("Client (pid %d, number %d) receive answer %c\n", argp->pid, argp->number, result);

	pthread_exit(&result);
}

int *
bakery_service_1_svc(struct REQUEST *argp, struct svc_req *rqstp)
{
	static int  result;
	void *tmp;
	int s;
	pthread_t thr;
	pthread_attr_t attr;
	s = pthread_attr_init(&attr);
	if (s != 0)
	{
		perror("pthread_attr_init");
		exit(1);
	}
	s = pthread_create(&thr, &attr, bakery_service, argp);
	if (s != 0)
	{
		perror("pthread_create");
		exit(1);
	}
	s = pthread_attr_destroy(&attr);
	if (s != 0)
	{
		perror("pthread_attr_destroy");
		exit(1);
	}
	s = pthread_join(thr, &tmp);
	if (s != 0)
	{
		perror("pthread_join");
		exit(1);
	}
	result = *(int *)tmp;
	return &result;
}


