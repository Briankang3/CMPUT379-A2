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

extern QUEUE Queue;
extern POOL pool;

void* new_work(void* arg);   // this is the producer function
void* consume_work(void* arg);   // this is the consumer function
void consumer(pthread_t* consumer_t);            // finds an available consumer thread
void producer(pthread_t* producer_t,int n);            // puts the new task to the queue

#endif