#include <iostream>
#include <string>
#include "lock.h"
#include "given.h"
using namespace std;

// Start measuring time
auto begin=chrono::high_resolution_clock::now();

void init(int N){
    if (sem_init(&io_lock,0,1)!=0) perror("unable to initialize io_lock");

    // initialize the semaphores of "Queue"
    if (sem_init(&Queue.mutex,0,1)!=0) perror("unable to initialize mutex");
    if (sem_init(&Queue.empty,0,0)!=0) perror("unable to initialize empty");
    if (sem_init(&Queue.full,0,N)!=0) perror("unable to initialize full");

    // initialize the semaphores in "pool"
    if (sem_init(&pool.empty,0,0)!=0) perror("unable to initilize empty");
    if (sem_init(&pool.mutex,0,1)!=0) perror("unable to initilize mutex");

    // initialize "Q" and "count"
    for (int i=0;i<2*N;i++) Queue.Q.push_back(0);
    Queue.count=0;

    pool.t_waiting=queue<int>();

    info.threads=vector<int>(N,0);

    return;
}

int main(int argc,char* argv[]){

    int N=*argv[0]-'0';
    cout<<"there are "<<N<<" threads.\n";

    vector<string> cmd;
    string input;
    while (cin>>input) cmd.push_back(input);

    // initialize the critical section
    init(N);

    // put N consumer threads into the waiting list
    pthread_t all_threads[N];
    for (int i=0;i<N;i++){
        pthread_t consumer_thread;
        all_threads[i]=consumer_thread;
        pool.t_waiting.push(i);
        sem_post(&pool.empty);
    }

    pthread_t producer_t;
    // handle the inputs
    for (string& s:cmd){
        if (s[0]=='T'){
            // puts the new task to the queue
            producer(&producer_t,s[1]-'0');
            // find a thread to finish the task
            consumer(all_threads);
        }

        else{
            info.sleep++;

            int n=s[1]-'0';
            // calculate elapsed time
            auto end=chrono::high_resolution_clock::now();
            auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);
            cout<<elapsed<<'    '<<"ID=0    "<<"  "<<' '<<"sleep "<<n<<'\n';

            Sleep(n);
        }
    }

    // wait for the producer thread to finish
    pthread_join(producer_t,nullptr);
    // wait for all consumer threads to finish
    for (int i=0;i<N;i++) pthread_join(all_threads[i],nullptr);

    // calculate elapsed time
    auto end=chrono::high_resolution_clock::now();
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(end-begin);

    // print out the summary 
    cout<<"summary:\n";
    cout<<"work:"<<info.work<<'\n';
    cout<<"ask:"<<info.ask<<'\n';
    cout<<"receive:"<<info.receive<<'\n';
    cout<<"sleep:"<<info.sleep<<'\n';
    // for each consumer thread
    for (int i=0;i<N;i++){
        cout<<"Thread "<<N<<":"<<info.threads[i]<<'\n';
    }
    cout<<"Transactions per second:"<<(float)info.complete/elapsed<<'\n';

    return 0;
}