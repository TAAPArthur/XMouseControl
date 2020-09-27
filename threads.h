
#ifndef MPX_THREADS
#define MPX_THREADS

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define THREAD_SIGNALER_INITIALIZER (ThreadSignaler) {.mutex = PTHREAD_MUTEX_INITIALIZER, .condVar = PTHREAD_COND_INITIALIZER}
/**
 * Abstracts the wait and notify thread semantics
 * This class is used to coordinate actions between to threads that have some shared state in an attempt to
 * prevent busy sleeps
 */
typedef struct ThreadSignaler {
    pthread_mutex_t mutex;
    pthread_cond_t  condVar;
    volatile bool ready;
} ThreadSignaler ;
/**
 * Runs func under a lock and then notifies one or all waiting threads
 *
 * @param func
 */
void signalThread(ThreadSignaler* signaler);
/**
 * Blocks until the thread is signaled and func returns true
 *
 * @param  func
 * @param noResetSignal if set this->ready will be cleared; Only matters if waiting on ready
 */
void justWait(ThreadSignaler* signaler);
/**
 * Either sleeps delay ms or blocks until signaled
 *
 * @param condition whether to sleep or block
 * @param delay the time to sleep if condition is true
 */
void sleepOrWait(ThreadSignaler* signaler, uint32_t condition, long delay);
/**
 * Runs method in a new thread
 * @param func the method to run
 */
void spawnThread(void* (*func)());
#endif
