#include "lock.h"
#include <cstdio>
using namespace std;

void* new_work(void* arg){
    // when a producer  new work
    QUEUE* q=(QUEUE*)arg;
    // waits when it's full
    sem_wait(q->full);

    // puts a job to "Q"

}

void* consume_work(void* arg){
    // when a thread starts running a work
    QUEUE* q=(QUEUE*)arg;
    // waits when it's empty
    sem_wait(q->empty);
}