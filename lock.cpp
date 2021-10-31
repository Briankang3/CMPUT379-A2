#include "lock.h"
#include "given.h"
using namespace std;

QUEUE Queue;
INFO info;
sem_t io_lock;
sem_t producing;

// this is the start routine of the producer thread
void* new_work(void* arg){
    int* q=(int*)arg;
    int n=*q;         // n is the parameter to pass to Trans()

    sem_wait(&info.wrt);
    info.work++;
    sem_post(&info.wrt);

    auto end=chrono::high_resolution_clock::now();
    float elapsed=chrono::duration_cast<chrono::milliseconds>(end - start).count();
    elapsed/=1000;

    // only one thread can access I/O stream simultaneously
    sem_wait(&io_lock);
    output<<elapsed<<"     "<<"ID=0    "<<"Q="<<Queue.count<<"   work "<<n<<'\n';
    sem_post(&io_lock);

    // waits when it's full
    sem_wait(&Queue.full);
    // wait when the critical section is occupied
    sem_wait(&Queue.mutex);     

    // puts a job to "Q"
    Queue.Q.push(n);
    Queue.count++;

    sem_post(&Queue.mutex);      // release the lock
    sem_post(&Queue.empty);      // ensures it is not empty

    return nullptr;
}

// this is the start routine of consumer threads
void* consume_work(void* arg){
    // when a thread starts running a work 
    int* q=(int*)arg;
    int n=*q;          // n is the thread id

    while (1){
        // now this process asks for work
        auto end=chrono::high_resolution_clock::now();
        float elapsed=chrono::duration_cast<chrono::milliseconds>(end - start).count();
        elapsed/=1000;

        sem_wait(&io_lock);
        output<<elapsed<<"     "<<"ID="<<n<<"    "<<"Q="<<Queue.count<<"   ask"<<'\n';
        sem_post(&io_lock);

        sem_wait(&info.wrt);
        info.ask++;
        sem_post(&info.wrt);

        int m;            // m will be the parameter to pass to Trans()
        sem_wait(&Queue.empty);     // waits when it's empty
        sem_wait(&Queue.mutex);     // wait when other threads are accessing the critical section

        // tackles the situation where the current threads get empted AFTER sem_wait(&empty) returns,
        // but after resuming, "Q" becomes empty again since the tasks have been taken by other threads
        if (Queue.Q.empty()){            
            sem_post(&Queue.mutex);
            sem_post(&Queue.full);

            continue;
        }

        finished[n-1]=false;        // it has yet finished the newly received task
        m=Queue.Q.front();
        Queue.Q.pop();
        Queue.count--;

        sem_post(&Queue.mutex);
        sem_post(&Queue.full);     // ensure it is not full

        sem_wait(&info.wrt);
        info.receive++;
        sem_post(&info.wrt);

        end=chrono::high_resolution_clock::now();
        elapsed=chrono::duration_cast<chrono::milliseconds>(end - start).count();
        elapsed/=1000;

        // access I/O stream
        sem_wait(&io_lock);
        output<<elapsed<<"     "<<"ID="<<n<<"    "<<"Q="<<Queue.count<<"   receive "<<m<<'\n';
        sem_post(&io_lock);

        Trans(m);        // Transaction

        finished[n-1]=true;      // current task finished

        sem_wait(&info.wrt);
        info.complete++;
        info.threads[n-1]++;
        sem_post(&info.wrt);

        end=chrono::high_resolution_clock::now();
        elapsed=chrono::duration_cast<chrono::milliseconds>(end - start).count();
        elapsed/=1000;

        sem_wait(&io_lock);
        output<<elapsed<<"     "<<"ID="<<n<<"    "<<Queue.count<<"   complete "<<m<<'\n';
        sem_post(&io_lock);

        if (done){       // when the end of file is reached
            sem_wait(&Queue.mutex);
            bool empty=Queue.Q.empty();
            sem_post(&Queue.mutex);

            if (empty){
                pthread_mutex_lock(&M);
                signaled=true;
                pthread_cond_signal(&END);       // signal the parent process that the task queue is empty, promting it to wait and cancel threads
                pthread_mutex_unlock(&M);

                break;
            }
        }
    }

    return nullptr;
}

// this function uses the only producer thread to put a new task on the queue
void producer(pthread_t* producer_t,int n){
    sem_wait(&producing);       // only one producer thread concurrently
    pthread_create(producer_t,nullptr,&new_work,&n);
    pthread_join(*producer_t,nullptr);       // wait for the running producer thread to terminate
    sem_post(&producing);      // another producer thread can be created

    return;
}