
#ifdef _WINDOWS
#include <windows.h>
#elif defined(_LINUX)
#include <unistd.h>
#include <pthread.h>
#endif

#include <stdio.h>

#include "seashell.h"
#include "thread_private.h"

namespace seashell
{

//Running/initialization states
const char THREAD_UNINIT = 0;
const char THREAD_RUNNING = 1;
const char THREAD_TERMINATE_REQUEST = 2;
//Termination states
const char THREAD_FINISHED = 3;
const char THREAD_FAILURE = 4;

namespace thread
{
    ThreadId getCurrentThreadId()
    {
#ifdef _WINDOWS
        return ThreadId(
          ::GetCurrentThreadId(), ::GetCurrentThread()
          );
#elif defined(_LINUX)
        return pthread_self();
#endif
    }



    sint getCurrentThreadNumericId()
    {
#ifdef _WINDOWS
        return (sint)GetCurrentThreadId();
#elif defined(_LINUX)
        return (sint)pthread_self();
#endif
    }



#ifdef _WINDOWS
    DWORD WINAPI threadStart(void* pThread)
    {
        ((Thread_Help*)pThread)->runThread();
        return 0;
    }
#elif defined(_LINUX)
    void* threadStart(void* pThread)
    {
        ((Thread_Help*)pThread)->runThread();
        return 0;
    }
#endif //Operating Systems
}//thread

Thread::Thread() :
  thread_current_(0)
{
}



Thread::~Thread()
{
    //If we have a current thread, delete it.  This will throw an exception if
    //the thread is still running.  Otherwise, it will just delete the 
    //Thread_Help object.
    delete thread_current_;
}



sint Thread::startThread()
{
    if (thread_current_)
        return 0;

    //Start the thread and return success
    thread_current_ = new Thread_Help(this);
    thread_current_->startThread();
    return 1;
}



void Thread::stopThread(sint wait)
{
    if (thread_current_) {
        thread_current_->haltThread(wait);
    }
}



void Thread::queryExit()
{
    eassert(thread_current_, Exception, "queryExit() called without a running "
      "thread.");

    //Have we been requested to terminate?
    if (thread_current_->state_ == THREAD_TERMINATE_REQUEST) {
        PROFILER_TERMINATE_THREAD();
        thread_current_->state_ = THREAD_FINISHED;
#if defined(_WINDOWS)
        ExitThread(0);
#elif defined(_LINUX)
        pthread_exit(0);
#endif
        ethrow(Exception, "Thread termination request did not succeed.");
    }
}



Thread_Help::Thread_Help(Thread* object)
    : state_(THREAD_UNINIT), object_(object)
{
}



void Thread_Help::startThread()
{
#ifdef _WINDOWS

    information_.handle = CreateThread(0, 0, thread::threadStart, this, 0, 
      &information_.id);

#elif defined(_LINUX)

    pthread_create(&information_, 0, thread::threadStart, this);

#endif //Operating systems
}



void Thread_Help::haltThread(sint wait)
{
    //Are we already terminated?
    if (state_ >= THREAD_FINISHED)
        return;

    //Ensure initialization is finished
    while (state_ < THREAD_RUNNING); //timing::sleepThread(0);

    //Signal termination
    if (state_ < THREAD_FINISHED) {
        state_ = THREAD_TERMINATE_REQUEST;
    }

    //Block?
    if (wait) {
        //Wait for the thread to exit peacefully
#ifdef _WINDOWS
        WaitForSingleObject(information_.handle, INFINITE);
        CloseHandle(information_.handle);
#elif defined(_LINUX)
        pthread_join(information_, 0);
#endif
    }
    else {
        //Terminate.
#ifdef _WINDOWS
        TerminateThread(information_.handle, 0);
        CloseHandle(information_.handle);
#elif defined(_LINUX)
        pthread_close(information_);
#endif
    }
}



Thread_Help::~Thread_Help()
{
    //Are we running?
    if (!isFinished()) {
        //Log exception
        elog(makeException("Threads must be terminated prior to their "
          "destruction."));

        //Force termination
#ifdef _WINDOWS
        TerminateThread(information_.handle, 0);
        CloseHandle(information_.handle);
#elif defined(_LINUX)
        pthread_close(information_);
#endif
    }
}



sint Thread_Help::isFinished()
{
    return (state_ == THREAD_FINISHED || state_ == THREAD_FAILURE);
}



sint Thread_Help::wasFailure()
{
    return (state_ == THREAD_FAILURE);
}



void Thread_Help::runThread()
{
    try {
        eassert(this == object_->thread_current_, Exception, "Thread set to "
            "execute run(), but is not the object's Thread object.");

        state_ = THREAD_RUNNING;
        object_->run();
        state_ = THREAD_FINISHED;
    }
    catch (const Exception& e) {
        elog(e);
        state_ = THREAD_FAILURE;
    }
    catch (...) {
        if (state_ != THREAD_FINISHED)  {
            //otherwise, valid thread termination
            elog(makeException("Unhandled, unknown exception caught."));
            state_ = THREAD_FAILURE;
        }
        PROFILER_TERMINATE_THREAD();
        throw;
    }
    PROFILER_TERMINATE_THREAD();
}

} //seashell



#if TESTING >= TESTLEVEL_THOROUGH
TEST_BUDDY(basicThreads)
    static seashell::Mutex iLock;
    static int i = 0;
    class testThread : public seashell::Thread
    {
    public:
        testThread()
        {
            startThread();
	        printf("Thread created\n");
        }

        ~testThread()
        {
#if PROFILE
            PROFILER(0); profiler::printStackTrace(stdout, profiler::getStackFingerprint());
#endif
			stopThread();
	        printf("Thread destroyed\n");
        }

        void run()
        {
            for (sint ii = 0; ii < 200000; ii++) {
                //Interestingly, even simple 
                //increments do not work without
                //mutexes.
                LockMutex(iLock);
                i++;
            }
        }
    };
    {
        testThread t[3];
        testThread t2;
    }
    testAssert(i == 800000, "Threads did not properly execute.  i = %i", i);

    EMBED_TEST_BUDDY(ErrorTypes)
    {
        class Test : public seashell::Thread {
            void run() 
            {
                timing::sleepThread(100);
            }
        };

        {
            printf("No logged exception:\n");
            Test t;
            t.startThread();
            t.startThread();
            t.stopThread();
        }

#if 0
        //Causes pure virtual call exceptions
        {
            PROFILER("You Should See An Exception");
            printf("Logged exception:\n");
            Test t;
            t.startThread();
        }
#endif
    }
    END_EMBED_TEST_BUDDY()

    EMBED_TEST_BUDDY(VirtualCallTests)
    {
        //Ensure that virtual calls are NOT screwed up if the constructor is
        //called.
        class A : public seashell::Thread {
        public:
            A()
            {
                a_ = 0;
            }

            virtual ~A()
            {
            }

            void run()
            {
                for (suint i = 0; i < 10000; i++) {
                    box();
                }
            }

            virtual void box() { MessageBox(0, "Error", "Error", MB_OK);  }

            suint a_;
        };

        class B : public A {
        public:
            void box()
            {
                a_--;
            }

            B()
            {
                startThread();
            }

            ~B()
            {
                stopThread();
            }
        };

        B b[16];
    }
    END_EMBED_TEST_BUDDY()
END_TEST_BUDDY()



TEST_BUDDY(allocatingThreads)
{
    //Test to make sure mmgr works with concurrently allocating and 
    //deallocating threads.  Should report 64*4*8192 = 2 megs.
    static seashell::Mutex ilock;
    class concurrentAllocations : public seashell::Thread
    {
    public:
        concurrentAllocations()
        {
            startThread();
        }

        ~concurrentAllocations()
        {
            stopThread();
        }

        void run()
        {
            PROFILER("Concurrent Allocations");

            static const sint numAllocation = 8192;
            sint* iarray[numAllocation];
            {//PROFILER("Allocating");
            printf("Allocating . . . \n");
            for (sint i = 0; i < numAllocation; i++) {
                iarray[i] = new sint (i);
            }}
            {//PROFILER("Deallocating");
            printf("De-allocating . . . \n");
            for (sint i = 0; i < numAllocation; i++) {
                delete iarray[i];
            }}
        }
    };

    {
        concurrentAllocations cArray[64];
    }
}
END_TEST_BUDDY()
#endif //TESTING
