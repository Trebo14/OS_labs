#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct Node {
	void *data;
	unsigned long seq;
	struct Node *next;
};

int monitor_init(Monitor *monitor) {
	if (monitor == NULL)
		return EINVAL;

	monitor->head = NULL;
	monitor->tail = NULL;
	monitor->next_seq = 0;

	int init_rc = pthread_mutex_init(&monitor->mutex, NULL);
	if (init_rc)
		return init_rc;

	init_rc = pthread_cond_init(&monitor->condition, NULL);
	if (init_rc) {
		int destroy_rc = pthread_mutex_destroy(&monitor->mutex);
		if (destroy_rc)
			return destroy_rc;
		return init_rc;
	}
	return 0;
}

int monitor_provide(Monitor *monitor, void *data) {
	if (monitor == NULL || data == NULL)
		return EINVAL;

	Node *newNode = malloc(sizeof *newNode);
	if (newNode == NULL)
		return ENOMEM;
	newNode->data = data;
	newNode->next = NULL;

	int lock_rc = pthread_mutex_lock(&monitor->mutex);
        if (lock_rc) {
                free(newNode);
		return lock_rc;
	}

	newNode->seq = ++monitor->next_seq;

	if (monitor->tail == NULL) {
		monitor->tail = newNode;
		monitor->head = newNode;
	}
	else {
		monitor->tail->next = newNode;
		monitor->tail = newNode;
	}

        printf("Producer: sent event #%lu (data=%p)\n", newNode->seq, data);

	int signal_rc = pthread_cond_signal(&monitor->condition);
	int unlock_rc = pthread_mutex_unlock(&monitor->mutex);
	if (unlock_rc)
		return unlock_rc;
	if (signal_rc)
		return signal_rc;
	return 0;
}


int monitor_consume(Monitor *monitor, void **data) {
	if (monitor == NULL || data == NULL)
		return EINVAL;

	int lock_rc = pthread_mutex_lock(&monitor->mutex);
        if (lock_rc)
                return lock_rc;

	while (monitor->head == NULL) {
		int wait_rc = pthread_cond_wait(&monitor->condition, &monitor->mutex);
		if (wait_rc != 0) {
            		int unlock_rc = pthread_mutex_unlock(&monitor->mutex);
            		if (unlock_rc != 0)
                		return unlock_rc;
        		return wait_rc;
        	}
	}

	Node *first = monitor->head;
	monitor->head = first->next;
	if (monitor->head == NULL)
		monitor->tail = NULL;

	*data = first->data;
	unsigned long seq = first->seq;
	free(first);
	printf("Consumer: received event #%lu (data=%p)\n", seq, *data);

        int unlock_rc = pthread_mutex_unlock(&monitor->mutex);
        if (unlock_rc)
                return unlock_rc;
	return 0;
}

int monitor_destroy(Monitor *monitor) {
	if (monitor == NULL)
        	return EINVAL;

    	int lock_rc = pthread_mutex_lock(&monitor->mutex);
	if (lock_rc)
        	return lock_rc;

	if (monitor->head != NULL || monitor->tail != NULL) {
		int unlock_rc = pthread_mutex_unlock(&monitor->mutex);
		return unlock_rc != 0 ? unlock_rc : EBUSY;
	}

	int unlock_rc = pthread_mutex_unlock(&monitor->mutex);
	if (unlock_rc)
        	return unlock_rc;
	int cond_rc = pthread_cond_destroy(&monitor->condition);
	if (cond_rc)
        	return cond_rc;
    	int mutex_rc = pthread_mutex_destroy(&monitor->mutex);
	if (mutex_rc)
        	return mutex_rc;
    	return 0;
}
