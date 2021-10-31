# CMPUT379-A2

Files:
    given.cpp & given.h: Provided on eclass, but I modified them in adaptation of c++.

    lock.h: Defines structures. Declares (external) global variables, and functions in lock.cpp.
    lock.cpp: Defines functions, including consumer and producer thread starting routines, as well as global variables declared in lock.h.

    main.cpp: Declares and defines routines to handle input and initialize global variables that are shared among all threads.

Instruction to compile and run:
    compile: $ make
    run: $ ./prodcon 3 <in.txt

Summary of approach to solve the problems:
1. QUEUE Queue stores the parameters to be passed to Trans(). This is guarded by three semaphores full, empty, and mutex, which respectively blocks the calling thread when it is empty, full, and Q being accessed by anothor thread.

2. INFO info stores information of the program execution. This is guarded by the semaphore wrt to ensure the integrity of data by allowing one thread to change data simultaneously.

3. Create N consumer threads that keep running until the end of input file has been reached AND all of them have finished their current tasks.

4. When the end of input file is reached, the parent process changes boolean variable "done" to "true", implying at lease one consumer processes to signal when they finish AND the task queue is empty. The signaling above is implemented and guarded by boolean variable "signaled", mutex M, and conditional variable END. 

Once the parent process receives the ending signal, it waits for all consumer threads to finish the current task before canceling all threads. The correctness of canceling threads is ensured by "vector<bool> finished".