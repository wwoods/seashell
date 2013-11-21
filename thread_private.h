//Walt Woods
//May 1st, 2008

namespace seashell
{

//Thread class for running actual threads.

//Thread class - creates a thread, running the run() routine of a Threaded 
//object.  On destruction of the Thread class, the spawned thread will 
//terminate with its next call to testTerminate().
class Thread_Help
{
    friend class Thread;
	
public:
    /**Default constructor.  Destructor will block for thread termination.
      *Thread is terminated on either its function exit or a call to 
      *testTerminate() in the thread routine after the destructor has been 
      *called.
      * @param object Object whose run() function will be called. */
    Thread_Help(Thread* object);

    /**Starts the thread. */
    void startThread();

    /**Halts the thread.
      * @param wait If non-zero, set state to terminate and block until the 
      *thread terminates.  If zero, terminate the thread forcefully. */
    void haltThread(sint wait);

    /**Destructor.  Throws an exception if the thread is not terminated. */
    ~Thread_Help();

    /** @return If the thread is still executing (has not yet halted), returns
      *0.  Otherwise, the return value is 1.
      */
    sint isFinished();

    /** @return If the thread finished because of an exception, returns 1.
      *Otherwise, the return value is 0.
      */
    sint wasFailure();

public:
    /**Runs the run() function and carries out thread operations.  Should not
      *be called manually.
      */
    void runThread();

private:
    /**Has the run() function finished or been run yet?
      */
    volatile sint state_;

    /**Since the parameters required to maintain a thread differ between 
      *platforms, the ThreadData structure holds the necessary information
      *and is defined in the source file port_thread.cpp.
      */
    ThreadId information_;

    /**The object containing the desired run() function.
      */
    Thread* object_;
};

} //seashell
