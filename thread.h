//Walt Woods
//August 5th, 2007
//Portable thread class to help with multithreaded applications

#ifndef THREAD_H_
#define THREAD_H_

namespace seashell
{

class Thread_Help;

//Derivable threaded object class.  When a Thread object is created that 
//references an instance of this object class, the function run() is 
//ran in a separate thread.  
//
//This class is VERY NECESSARY.  Since object initialization and shutdown has
//parent objects scoped to the outside of an object's life, having a direct
//Thread object results in an object being destructed while its thread is
//still operating on its data.
class Thread
{
    friend class Thread_Help;

public:
    /**Basic constructor. */
    Thread();

    /**Destructor - throws exception if the thread is still running, as this is
      *a dangerous situation and should not be necessary. */
    virtual ~Thread();

    /**Executed when a thread_start() is called.
      * @param data Additional data that was passed to the Thread object's
      *constructor.
      * @param thread Calling thread.  Important when used with testTerminate.
      */
    virtual void run() = 0;

    /**Starts execution of this thread.  DO NOT CALL THIS IN THE CONSTRUCTOR if
      *the class has any virtual functions.
      * @return Returns non-zero if the operation was successful.  If there
      *is already a thread running (or that has ran) this object, then the 
      *return value is zero and no action is performed. */
    sint startThread();

    /**Halts execution of the thread.  May be called any number of times 
      *without an error.  Should be called in ALL destructors, even for 
      *derivatives.
      * @param wait If non-zero, this call blocks until the thread terminates.
      *If zero, this call terminates the thread immediately. */
    void stopThread(sint wait = 1);

    /**Very important for any thread that will halt execution in Thread's 
      *destructor (or a thread that typically uses an infinite while loop).  
      *Call this function at any point when a valid termination may take 
      *place.
      * @param thread Thread to check exit status.  Obtained from 
      *run::thread.
      */
    void queryExit();

private:
    //The thread currently executing this object's procedure
    Thread_Help* thread_current_;
};



namespace thread
{
    /** @return Returns the currently executing thread's identifier.  May be used
      *for various thread-specific purposes.
      *
      *In Windows, the HANDLE returned will ONLY ever reference the thread that 
      *attempts to pass that handle to any function.  Use OpenThread() with the 
      *id to access an actual handle to the current thread at a later execution
      *point.
      */
    ThreadId getCurrentThreadId();

    /** @return Returns the thread id in a printable, integer format.
      */
    sint getCurrentThreadNumericId();
} //thread

} //seashell

#endif//THREAD_H_
