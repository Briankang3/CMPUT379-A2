#include <iostream>
#include <string>
#include <pthread.h>
#include "lock.h"
#include "given.h"
using namespace std;

QUEUE Queue;

void init(int N){
    // initialize three semaphores
    if (sem_init(&Queue.mutex,0,1)!=0) perror("unable to initialize mutex");
    if (sem_init(&Queue.empty,0,0)!=0) perror("unable to initialize empty");
    if (sem_init(&Queue.full,0,N)!=0) perror("unable to initialize full");

    // initialize "Q" and "count"
    for (int i=0;i<2*N;i++) Queue.Q.append(0);
    Queue.count=0;
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
        if (pthread_create(&consumer_thread,nullptr,&new_work,&Queue)!=0) perror("fail to create a consumer thread");
        all_threads[i]=consumer_thread;
    }

    return 0;
}