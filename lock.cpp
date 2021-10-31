#include "lock.h"
#include "given.h"
using namespace std;

QUEUE Queue;
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
    int n=*q;          

    while (!done){
        // now this process asks for work
        sem_wait(&io_lock);
        cout<<"     "<<"ID="<<n<<"    "<<"Q="<<Queue.count<<"   ask"<<'\n';
        sem_post(&io_lock);

        sem_wait(&info.wrt);
        info.ask++;
        sem_post(&info.wrt);

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
        cout<<"     "<<"ID="<<n<<"    "<<"Q="<<Queue.count<<"   receive "<<m<<'\n';
        sem_post(&io_lock);

        Trans(m);

        sem_wait(&info.wrt);
        info.complete++;
        info.threads[n-1]++;
        sem_post(&info.wrt);

        sem_wait(&io_lock);
        cout<<"     "<<"ID="<<n<<"    "<<Queue.count<<"   complete "<<m<<'\n';
        sem_post(&io_lock);
    }

    return nullptr;
}

void producer(pthread_t* producer_t,int n){
    // this function uses the only producer thread to put a new task on the queue

    sem_wait(&producing);
    pthread_create(producer_t,nullptr,&new_work,&n);
    pthread_join(*producer_t,nullptr);
    sem_post(&producing);

    return;
}