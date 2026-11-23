#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#define EVENT_COUNT 5


typedef struct {
    int id;
    double value;
} Event;


static void die(const char *what, int rc) {
    if (rc != 0) {
        fprintf(stderr, "%s: %s\n", what, strerror(rc));
        exit(EXIT_FAILURE);
    }
}


void *producer(void *arg) {
    Monitor *monitor = arg;

    for (int i = 1; i <= EVENT_COUNT; ++i) {
        sleep(1);

        Event *event = malloc(sizeof *event);
        assert(event != NULL);

        event->id = i;
        event->value = i * 1.5;

        int rc = monitor_provide(monitor, event);
        die("monitor_provide", rc);
    }
    return NULL;
}


void *consumer(void *arg) {
    Monitor *monitor = arg;

    for (int i = 1; i <= EVENT_COUNT; ++i) {
        void *data = NULL;

        int rc = monitor_consume(monitor, &data);
        die("monitor_consume", rc);

        Event *event = data;
        printf("Consumer: processed event #%d, value = %.1f\n", event->id, even>
        free(event);
    }
    return NULL;
}


int main(void) {
    Monitor monitor;

    int rc = monitor_init(&monitor);
    die("monitor_init", rc);

    pthread_t producer_thread;
    pthread_t consumer_thread;

    rc = pthread_create(&consumer_thread, NULL, consumer, &monitor);
    die("pthread_create", rc);

    rc = pthread_create(&producer_thread, NULL, producer, &monitor);
    die("pthread_create", rc);

    rc = pthread_join(producer_thread, NULL);
    die("pthread_join", rc);

    rc = pthread_join(consumer_thread, NULL);
    die("pthread_join", rc);

    rc = monitor_destroy(&monitor);
    die("monitor_destory", rc);

    return 0;
}
