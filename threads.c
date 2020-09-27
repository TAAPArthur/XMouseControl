#include <pthread.h>
#include "threads.h"


void signalThread(ThreadSignaler* condMutex) {
    pthread_mutex_lock(&condMutex->mutex);
    condMutex->ready = 1;
    pthread_mutex_unlock(&condMutex->mutex);
    pthread_cond_signal(&condMutex->condVar);
}
void justWait(ThreadSignaler* condMutex) {
    pthread_mutex_lock(&condMutex->mutex);
    while(!condMutex->ready) {
        pthread_cond_wait(&condMutex->condVar, &condMutex->mutex);
    }
    condMutex->ready = 0;
    pthread_mutex_unlock(&condMutex->mutex);
}

void msleep(long ms) {
    struct timespec duration = {
        .tv_sec = ms / 1000,
        .tv_nsec = ((ms % 1000) * 1e6),
    };
    nanosleep(&duration, NULL);
}
void sleepOrWait(ThreadSignaler* signaler, uint32_t condition, long delay) {
    if(condition)
        msleep(delay);
    else
        justWait(signaler);
}

pthread_t _spawnThread(void* (*func)()) {
    pthread_t thread;
    pthread_create(&thread, NULL, func, NULL);
    return thread;
}
void spawnThread(void* (*func)()) {
    pthread_detach(_spawnThread(func));
}
