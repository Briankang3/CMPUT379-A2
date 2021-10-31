#include <iostream>
#include <string>
#include <sstream>
#include "lock.h"
#include "given.h"
using namespace std;

bool done=false;
bool signaled=false;
pthread_cond_t END=PTHREAD_COND_INITIALIZER;
pthread_mutex_t M=PTHREAD_MUTEX_INITIALIZER;
vector<bool> finished;

void init(int N){
    if (sem_init(&io_lock,0,1)!=0) perror("unable to initialize io_lock");
    if (sem_init(&producing,0,1)!=0) perror("unable to initialize producing");
    if (sem_init(&info.wrt,0,1)!=0) perror("unable to initialize wrt");

    // initialize the semaphores of "Queue"
    if (sem_init(&Queue.mutex,0,1)!=0) perror("unable to initialize mutex");
    if (sem_init(&Queue.empty,0,0)!=0) perror("unable to initialize empty");
    if (sem_init(&Queue.full,0,N)!=0) perror("unable to initialize full");

    // initialize "Q" and "count"
    Queue.Q=queue<int>();
    Queue.count=0;

    for (int i=0;i<N;i++){
        info.threads.push_back(0);
        finished.push_back(false);
    }

    return;
}

int main(int argc,char* argv[]){

    int N;
    istringstream iss(argv[1]);
    iss>>N;

    vector<string> cmd;
    string input;
    while (cin>>input) cmd.push_back(input);

    // initialize the critical section
    init(N);

    // create N consumer threads
    pthread_t all_threads[N];
    int thread_id[N];
    for (int i=0;i<N;i++){
        pthread_t consumer_thread;
        all_threads[i]=consumer_thread;

        thread_id[i]=i+1;
    }

    for (int i=0;i<N;i++) pthread_create(all_threads+i,nullptr,&consume_work,thread_id+i);

    pthread_t producer_t;
    // handle the inputs
    for (string& s:cmd){
        if (s[0]=='T'){
            // puts the new task to the queue
            producer(&producer_t,s[1]-'0');
        }

        else{
            sem_wait(&info.wrt);
            info.sleep++;
            sem_post(&info.wrt);

            int n=s[1]-'0';

            sem_wait(&io_lock);
            cout<<"     "<<"ID=0    "<<"  "<<' '<<"sleep "<<n<<'\n';
            sem_post(&io_lock);

            Sleep(n);
        }
    }
    
    done=true;

    // wait for the producer thread to finish
    pthread_join(producer_t,nullptr);

    // wait for all consumer threads to finish
    pthread_mutex_lock(&M);

    while (!signaled) pthread_cond_wait(&END,&M);    
    pthread_mutex_unlock(&M);

    // wait for consumer threads to finish their current task, then terminate
    while (1){
        bool terminate=true;
        for (int i=0;i<N;i++){
            if (finished[i]==false){
                terminate=false;
                break;
            }

            else pthread_cancel(all_threads[i]);
        }

        if (terminate) break;
    }

    sem_destroy(&io_lock);
    sem_destroy(&info.wrt);
    sem_destroy(&producing);
    sem_destroy(&Queue.full);
    sem_destroy(&Queue.empty);
    sem_destroy(&Queue.mutex);

    // print out the summary 
    cout<<"summary:\n";
    cout<<"work:"<<info.work<<'\n';
    cout<<"ask:"<<info.ask<<'\n';
    cout<<"receive:"<<info.receive<<'\n';
    cout<<"complete:"<<info.complete<<'\n';
    cout<<"sleep:"<<info.sleep<<'\n';
    // for each consumer thread
    for (int i=0;i<N;i++){
        cout<<"Thread "<<i+1<<":"<<info.threads[i]<<'\n';
    }
    cout<<"Transactions per second:"<<'\n';

    return 0;
}