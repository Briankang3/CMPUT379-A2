#ifndef _lockh_
#define _lockh_

#include <semaphore.h>
#include <vector>

struct QUEUE{
    sem_t mutex;     // Only one thread can access "TransSave" simutaneously.
    sem_t full;      // blocks producer when the Q is full
    sem_t empty;     // blocks consumer when the Q is empty
    vector<int> Q;   // "int" is the number of seconds
    int count;       // the actual number of jobs available
}

void init(int N);    // initialize all three semephores and "Q"
void* new_work(void* arg);   // this is the producer function
void* consume_work(void* arg);   // this is the consumer function

#endif