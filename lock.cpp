#include "lock.h"
#include "given.h"
using namespace std;

QUEUE Queue;
POOL pool;
INFO info;
sem_t io_lock;
sem_t producing;

void* new_work(void* arg){
    // when a producer receives new work
    int* q=(int*)arg;
    int n=*q;

    sem_wait(&info.wrt);
    info.work++;
    sem_post(&info.wrt);

    // only one thread can access I/O stream simultaneously
    sem_wait(&io_lock);
    cout<<"     "<<"ID=0    "<<"Q="<<Queue.count<<"   work "<<n<<'\n';
    sem_post(&io_lock);

    // waits when it's full
    sem_wait(&Queue.full);
    // wait when the critical section is occupied
    sem_wait(&Queue.mutex);     

    // puts a job to "Q"
    Queue.Q.push(n);
    Queue.count++;

    sem_post(&Queue.mutex);
    sem_post(&Queue.empty);

    return nullptr;
}

void* consume_work(void* arg){
    // when a thread starts running a work 
    int* q=(int*)arg;
    int n=*q;           // WARNING: something might be wrong with this. Needs fixing before submission.

    cout<<"Test: n="<<n<<'\n';

    // waits when it's empty
    int m;
    sem_wait(&Queue.empty);
    // wait when other threads are accessing the critical section
    sem_wait(&Queue.mutex);

    m=Queue.Q.front();
    Queue.Q.pop();
    Queue.count--;

    sem_post(&Queue.mutex);
    sem_post(&Queue.full);

    sem_wait(&info.wrt);
    info.receive++;
    sem_post(&info.wrt);

    // access I/O stream
    sem_wait(&io_lock);
    cout<<"     "<<"ID="<<n+1<<"    "<<"Q="<<Queue.count<<"   receive "<<m<<'\n';
    sem_post(&io_lock);

    Trans(m);

    // update "pool"
    sem_wait(&pool.mutex);

    pool.t_waiting.push(n);

    sem_post(&pool.empty);   
    sem_post(&pool.mutex);

    sem_wait(&info.wrt);
    info.complete++;
    sem_post(&info.wrt);

    sem_wait(&io_lock);
    cout<<"     "<<"ID="<<n+1<<"    "<<Queue.count<<"   complete "<<m<<'\n';
    sem_post(&io_lock);

    return nullptr;
}

void consumer(pthread_t* consumer_t){
    // this function tries to run a task in the queue with an available consumer thread
    // if no consumer threads are available, it will wait
    // waits when no consumer threads are available
    int n;

    sem_wait(&pool.empty);
    // enters "pool"
    sem_wait(&pool.mutex);
    // acquire an available thread
    n=pool.t_waiting.front();
    pool.t_waiting.pop();
    sem_post(&pool.mutex);

    sem_wait(&info.wrt);
    info.ask++;
    info.threads[n]++;
    sem_post(&info.wrt);

    // access I/O stream
    sem_wait(&io_lock);
    cout<<"     "<<"ID="<<n+1<<"    "<<"Q="<<Queue.count<<"   ask"<<'\n';
    sem_post(&io_lock);

    pthread_create(&consumer_t[n],nullptr,&consume_work,&n);

    return;
}

void producer(pthread_t* producer_t,int n){
    // this function uses the only producer thread to put a new task on the queue

    sem_wait(&producing);
    pthread_create(producer_t,nullptr,&new_work,&n);
    pthread_join(*producer_t,nullptr);
    sem_post(&producing);

    return;
}