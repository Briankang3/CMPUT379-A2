#include <iostream>
#include <string>
#include <sstream>
#include "lock.h"
#include "given.h"
using namespace std;

chrono::high_resolution_clock::time_point start;
fstream output;
bool done=false;      // changes when the end of file is reached
bool signaled=false;
pthread_cond_t END=PTHREAD_COND_INITIALIZER;
pthread_mutex_t M=PTHREAD_MUTEX_INITIALIZER;
vector<bool> finished;     // indicates whether a consumer thread has finished its currently running task


// Initialize the program.
// Input: N, the number of consumer threads
void init(int N){
    start=chrono::high_resolution_clock::now();

    if (sem_init(&io_lock,0,1)!=0) perror("unable to initialize io_lock");        // sephophore preventing I/O stream being garbled.
    if (sem_init(&producing,0,1)!=0) perror("unable to initialize producing");    // only one producer thread
    if (sem_init(&info.wrt,0,1)!=0) perror("unable to initialize wrt");           // only one consumer thread shall access info

    // initialize the semaphores of "Queue"
    if (sem_init(&Queue.mutex,0,1)!=0) perror("unable to initialize mutex");
    if (sem_init(&Queue.empty,0,0)!=0) perror("unable to initialize empty");
    if (sem_init(&Queue.full,0,2*N)!=0) perror("unable to initialize full");

    // initialize "Q" and "count"
    Queue.Q=queue<int>();
    Queue.count=0;

    for (int i=0;i<N;i++){
        info.threads.push_back(0);
        finished.push_back(false);     // since no consumer threads have been created, none of them have finished their current job
    }

    return;
}

int main(int argc,char* argv[]){

    int N;
    istringstream iss(argv[1]);
    iss>>N;

    string filename;
    if (argc>2 && argv[2][0]!='<') filename="prodcon."+(string)argv[2]+".log";
    else filename="prodcon.log";

    output.open(filename,ios::out);

    vector<string> cmd;
    string input;
    while (cin>>input) cmd.push_back(input);

    // initialize the program
    init(N);

    // stores information of consumer thread handles
    pthread_t all_threads[N];
    int thread_id[N];
    for (int i=0;i<N;i++){
        pthread_t consumer_thread;
        all_threads[i]=consumer_thread;

        thread_id[i]=i+1;
    }
    // create all consumer threads
    for (int i=0;i<N;i++) pthread_create(all_threads+i,nullptr,&consume_work,thread_id+i);

    pthread_t producer_t;      // handle for the producer thread
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

            auto end=chrono::high_resolution_clock::now();
            float elapsed=chrono::duration_cast<chrono::milliseconds>(end - start).count();
            elapsed/=1000;

            sem_wait(&io_lock);
            output<<elapsed<<"     "<<"ID=0    "<<"  "<<' '<<"sleep "<<n<<'\n';
            sem_post(&io_lock);

            Sleep(n);
        }
    }
    // the end of input file has been reached
    done=true;

    // wait for the producer thread to finish
    pthread_join(producer_t,nullptr);

    // wait for the signal indicating the task queue is empty
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

            else pthread_cancel(all_threads[i]);     // when all consumer threads are free
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
    output<<"summary:\n";
    output<<"work:"<<info.work<<'\n';
    output<<"ask:"<<info.ask<<'\n';
    output<<"receive:"<<info.receive<<'\n';
    output<<"complete:"<<info.complete<<'\n';
    output<<"sleep:"<<info.sleep<<'\n';
    // for each consumer thread
    for (int i=0;i<N;i++){
        output<<"Thread "<<i+1<<":"<<info.threads[i]<<'\n';
    }

    auto end=chrono::high_resolution_clock::now();
    float elapsed=chrono::duration_cast<chrono::milliseconds>(end - start).count();
    float average=1000*(float)info.complete/elapsed;

    output<<"Transactions per second:"<<average<<'\n';

    output.close();

    return 0;
}