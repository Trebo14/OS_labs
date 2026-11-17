#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#define EVENT_COUNT 5

typedef struct {
    int id;
    double value;
} Event;

void *producer(void *arg) {
    Monitor *monitor = arg;

    for (int i = 1; i <= EVENT_COUNT; ++i) {
        sleep(1);

        Event *event = malloc(sizeof *event);
        assert(event != NULL);

        event->id = i;
        event->value = i * 1.5;

        int rc = monitor_provide(monitor, event);
        assert(rc == 0);
    }

    return NULL;
}

void *consumer(void *arg) {
    Monitor *monitor = arg;

    for (int i = 1; i <= EVENT_COUNT; ++i) {
        void *data = NULL;

        int rc = monitor_consume(monitor, &data);
        assert(rc == 0);

        Event *event = data;
        printf("Consumer: processed event #%d, value = %.1f\n", event->id, event->value);
        free(event);
    }

    return NULL;
}

int main(void) {
    Monitor monitor;

    int rc = monitor_init(&monitor);
    assert(rc == 0);

    pthread_t producer_thread;
    pthread_t consumer_thread;

    rc = pthread_create(&consumer_thread, NULL, consumer, &monitor);
    assert(rc == 0);

    rc = pthread_create(&producer_thread, NULL, producer, &monitor);
    assert(rc == 0);

    rc = pthread_join(producer_thread, NULL);
    assert(rc == 0);

    rc = pthread_join(consumer_thread, NULL);
    assert(rc == 0);

    rc = monitor_destroy(&monitor);
    assert(rc == 0);

    return 0;
}
