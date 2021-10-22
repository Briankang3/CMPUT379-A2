#include <iostream>
#include <string>
#include <pthread.h>
#include <queue>
#include "lock.h"
#include "given.h"
using namespace std;

QUEUE Queue;

// Start measuring time
auto begin=chrono::high_resolution_clock::now();

void init(int N){
    // initialize three semaphores
    if (sem_init(&Queue.mutex,0,1)!=0) perror("unable to initialize mutex");
    if (sem_init(&Queue.empty,0,0)!=0) perror("unable to initialize empty");
    if (sem_init(&Queue.full,0,N)!=0) perror("unable to initialize full");

    // initialize "Q" and "count"
    for (int i=0;i<2*N;i++) Queue.Q.push_back(0);
    Queue.count=0;

    return;
}

int main(int argc,char* argv[]){

    int N=*argv[0]-'0';
    cout<<"there are "<<N<<" threads.\n";

    vector<string> cmd;
    string input;
    while (cin>>input) cmd.push_back(input);

    // create threads
    pthread_t all_threads[N];
    for (int i=0;i<N;i++){
        pthread_t consumer_thread;
        all_threads[i]=consumer_thread;
        Queue.t_waiting.push_back(i);
        if (pthread_create(&consumer_thread,nullptr,&consume_work,&Queue)!=0) perror("fail to create a consumer thread");
    }

    for (string& s:cmd){
        if (s[0]=='T'){
            sem_wait(Queue.mutex);
            Queue.Q.push_back(s[1]-'0');
            count++;
            sem_post(Queue.mutex);
        }

        else{
            Sleep(s[1]-'0');
        }
    }

    return 0;
}