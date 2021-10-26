#include "lock.h"
#include "given.h"
using namespace std;

QUEUE Queue;
POOL pool;
INFO info;
sem_t io_lock;

void* new_work(void* arg){
    // when a producer receives new work
    int* q=(int*)arg;
    int n=*q;

    info.work++;

    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);
    // only one thread can access I/O stream simultaneously
    sem_wait(&io_lock);
    cout<<elapsed<<'    '<<"ID=0    "<<"Q="<<Queue.count<<"work "<<n<<'\n';
    sem_post(&io_lock);

    // waits when it's full
    sem_wait(&Queue->full);
    // wait when the critical section is occupied
    sem_wait(&Queue->mutex);     

    // puts a job to "Q"
    Queue.Q.push_back(n);
    Queue.count++;

    sem_post(&Queue->mutex);
    sem_post(&Queue->empty);

    return nullptr;
}

void* consume_work(void* arg){
    // when a thread starts running a work
    int* q=(int*)arg;
    int n=*q;
    
    // waits when it's empty
    sem_wait(&Queue->empty);
    // wait when other threads are accessing the critical section
    sem_wait(&Queue->mutex);

    info.receive++;

    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);

    // access I/O stream
    sem_wait(&io_lock);
    cout<<elapsed<<'    '<<"ID="<<n<<"    "<<"Q="<<Queue.count<<"receive "<<n<<'\n';
    sem_post(&io_lock);

    Trans(n);

    // update "pool"
    sem_wait(&pool.mutex);

    pool.t_waiting.push(n);
    sem_post(&pool.empty);

    sem_post(&pool.mutex);

    sem_post(&Queue->mutex);
    sem_post(&Queue->full);

    info.complete++;
    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);
    
    sem_wait(&io_lock);
    cout<<elapsed<<'    '<<"ID="<<n<<"    "<<Queue.count<<"complete "<<n<<'\n';
    sem_post(&io_lock);

    return nullptr;
}

void consumer(pthread_t* consumer_t){
    // this function tries to run a task in the queue with an available consumer thread
    // if no consumer threads are available, it will wait
    info.ask++;
    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);
    // access I/O stream
    sem_wait(&io_lock);
    cout<<elapsed<<'    '<<"ID=0    "<<"Q="<<Queue.count<<"ask"<<'\n';
    sem_wait(&io_lock);

    // waits when no consumer threads are available
    sem_wait(&pool.empty);
    // enters "pool"
    sem_wait(&pool.mutex);
    // acquire an available thread
    int n=pool.t_waiting.front();
    pool.t_waiting.pop();

    info.threads[n]++;

    sem_post(&pool.mutex);

    pthread_create(&consumer_t[n],nullptr,&consume_work,&n);

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