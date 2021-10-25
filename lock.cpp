#include "lock.h"
using namespace std;

QUEUE Queue;
POOL pool;

void* new_work(void* arg){
    // when a producer receives new work
    int* q=(int*)arg;
    int n=*q;

    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);
    cout<<elapsed<<'    '<<"ID=0    "<<"Q="<<Queue.count<<"work "<<n<<'\n';

    // waits when it's full
    sem_wait(Queue->full);
    // wait when the critical section is occupied
    sem_wait(Queue->mutex);     

    // puts a job to "Q"
    Queue.Q.push_back(n);
    Queue.count++;

    sem_post(Queue->mutex);
    sem_post(Queue->empty);

    return nullptr;
}

void* consume_work(void* arg){
    // when a thread starts running a work
    int* q=(int*)arg;
    int n=*q;
    
    // waits when it's empty
    sem_wait(Queue->empty);
    // wait when no 
    sem_wait(Queue->mutex);

    sem_post(Queue->mutex);
    sem_post(Queue->full);

    return nullptr;
}

void consumer(pthread_t* consumer_t){
    // this function tries to run a task in the queue with an available consumer thread
    // if no consumer threads are available, it will wait

    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);
    cout<<elapsed<<'    '<<"ID=0    "<<"Q="<<Queue.count<<"ask"<<'\n';


    return;
}

void producer(pthread_t* producer_t,int n){
    // this function uses the only producer thread to put a new task on the queue

    // wait for the running producer thread to finish
    pthread_join(*producer_t);
    // recycle the thread
    pthread_create(producer_t,nullptr,&new_work,&n);

    return;
}