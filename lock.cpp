#include "lock.h"
#include <cstdio>
using namespace std;

void* new_work(void* arg){
    // when a producer  new work
    QUEUE* q=(QUEUE*)arg;
    // waits when it's full
    sem_wait(q->full);

    sem_wait(q->mutex);     // wait when the critical section is occupied
    // puts a job to "Q"


    sem_post(q->mutex);
    sem_post(q->empty);

    return nullptr;
}

void* consume_work(void* arg){
    // when a thread starts running a work
    QUEUE* q=(QUEUE*)arg;
    // waits when it's empty
    sem_wait(q->empty);

    sem_wait(q->mutex);     // wait when the critical section is occupied



    sem_post(q->mutex);
    sem_post(q->full);

    return nullptr;
}