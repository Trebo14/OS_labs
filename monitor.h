#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>

typedef struct Node Node;
typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t condition;

	Node *head,
	     *tail;

	unsigned long next_seq;
} Monitor;

int monitor_init(Monitor *monitor);
int monitor_destroy(Monitor *monitor);

int monitor_provide(Monitor *monitor, void *data);
int monitor_consume(Monitor *monitor, void **data);

#endif
