#ifndef _lockh_
#define _lockh_

#include <semaphore.h>
#include <vector>
#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <queue>
#include <chrono>
using namespace std;

struct QUEUE{
    sem_t mutex;     // Only one thread can access QUEUE simutaneously.
    sem_t full;      // blocks producer when the Q is full
    sem_t empty;     // blocks consumer when the Q is empty
    queue<int> Q;   // "int" is the number of seconds for "transaction"
    int count;       // the actual number of jobs available
};

struct INFO{         // stores information of program execution
    int work=0;
    int ask=0;
    int receive=0;
    int complete=0;
    int sleep=0;
    vector<int> threads;   // the number of jobs each consumer thread has finished
    sem_t wrt;       // allow one thread to access memory
};

extern QUEUE Queue;
extern INFO info;
extern sem_t io_lock;      // one thread can access I/O stream simultaneouly
extern sem_t producing;    // equavalently only one producer thread
extern bool done;          // changes when the end of input file is reached
extern bool signaled;      // for sending a signal from a consumer thread to the parent
extern pthread_cond_t END; 
extern pthread_mutex_t M;
extern vector<bool> finished;
extern fstream output;
extern chrono::high_resolution_clock::time_point start;    // starting time

void* new_work(void* arg);   // this is the producer function
void* consume_work(void* arg);   // this is the consumer function
void producer(pthread_t* producer_t,int n);            // puts the new task to the queue

#endif