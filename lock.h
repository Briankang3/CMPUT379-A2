#ifndef _lockh_
#define _lockh_

#include <semaphore.h>
#include <vector>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <pthread.h>
#include <queue>
using namespace std;

extern chrono::time_point<std::chrono::high_resolution_clock> begin;    // beginning time of the program

struct QUEUE{
    sem_t mutex;     // Only one thread can access QUEUE simutaneously.
    sem_t full;      // blocks producer when the Q is full
    sem_t empty;     // blocks consumer when the Q is empty
    vector<int> Q;   // "int" is the number of seconds for "transaction"
    int count;       // the actual number of jobs available
};

struct POOL{
    sem_t mutex;     // only one thread can access the thread pool simultaneously
    sem_t empty;     // blocks consumer when t_waiting is empty
    queue<int> t_waiting;     // indicates which consumer threads are waiting for new work
};

struct INFO{         // stores information of program execution
    int work=0;
    int ask=0;
    int receive=0;
    int complete=0;
    int sleep=0;
    vector<int> threads;
};

extern QUEUE Queue;
extern POOL pool;
extern INFO info;
extern sem_t io_lock;      // one thread can access I/O stream simultaneouly

void* new_work(void* arg);   // this is the producer function
void* consume_work(void* arg);   // this is the consumer function
void consumer(pthread_t* consumer_t);            // finds an available consumer thread
void producer(pthread_t* producer_t,int n);            // puts the new task to the queue

#endif