
#ifdef _WINDOWS
#define _WIN32_WINNT 0x8000
#include <windows.h>
#endif

#include <stdio.h>

#include "seashell.h"

namespace seashell
{

MutexLocker::MutexLocker(Mutex& lock)
  : lock_(&lock)
{
    lock_->mutexLock();
}



MutexLocker::~MutexLocker()
{
    lock_->mutexUnlock();
}



SoftMutexLocker::SoftMutexLocker(Mutex& lock)
  : lock_(&lock)
{
    lock_->mutexSoftLock();
}



SoftMutexLocker::~SoftMutexLocker()
{
    lock_->mutexSoftUnlock();
}



Mutex::Mutex()
  : mutexSoftLocks_(0), mutexWantsHardlock_(0), mutexLocks_(0)
{
#ifdef _WINDOWS
    InitializeCriticalSection(&mutex_);
#elif defined(_LINUX)
    pthread_mutex_init(&mutex_, 0);
#endif //operating systems
}



Mutex::~Mutex()
{
#ifdef _WINDOWS
    DeleteCriticalSection(&mutex_);
#elif defined(_LINUX)
    pthread_mutex_destroy(&mutex_);
#endif //operating systems

    eassert(mutexLocks_ == 0, Exception, "Mutex destruction event when mutex "
      "is still owned by a thread!");
}



char Mutex::mutexIsLocked()
{
    if (mutexLocks_ > 0 || mutexSoftLocks_ > 0)
        return 1;
    return 0;
}



void Mutex::mutexLock_()
{
#ifdef _WINDOWS
    EnterCriticalSection(&mutex_);
#elif defined(_LINUX)
    pthread_mutex_lock(&mutex_);
#endif //operating systems

    eassert(mutexLocks_ >= 0, Exception, "Mutex lock count is negative.");
    eassert(mutexLocks_ == 0, Exception, "Mutex already locked.");
    mutexLocks_++;
}



sint Mutex::mutexTryLock_()
{
    sint success = 0;
#ifdef _WINDOWS
    if (TryEnterCriticalSection(&mutex_))
        success = 1;
#elif defined(_LINUX)
    if (pthread_mutex_trylock(&mutex_) == 0)
        success = 1;
#endif //operating systems

    if (success) {
        eassert(mutexLocks_ >= 0, Exception, "Mutex lock count is negative.");
        eassert(mutexLocks_ == 0, Exception, "Mutex already locked.");
        mutexLocks_++;
    }

    return success;
}



void Mutex::mutexUnlock_()
{
    eassert(mutexLocks_ >= 1, Exception, "Mutex unlock requested when mutex "
      "is not locked.");
    eassert(mutexLocks_ == 1, Exception, "Mutex illegally possesses multiple locks.");
    mutexLocks_--;

#ifdef _WINDOWS
    LeaveCriticalSection(&mutex_);
#elif defined(_LINUX)
    pthread_mutex_unlock(&mutex_);
#endif //operating systems
}



void Mutex::mutexLock()
{
    mutexWantsHardlock_ = 1;
    while (1) {
        mutexLock_();
        if (mutexSoftLocks_ == 0) {
            mutexWantsHardlock_ = 0;
            return;
        }
        mutexUnlock_();
    }
}



sint Mutex::mutexTryLock()
{
    sint result = mutexTryLock_();
    if (!result)
        return 0;
    if (mutexSoftLocks_ > 0) {
        mutexUnlock_();
        return 0;
    }
    return 1;
}



void Mutex::mutexUnlock()
{
    mutexUnlock_();
}



void Mutex::mutexSoftLock()
{
    while (mutexWantsHardlock_)
        timing::sleepThread(0);
    mutexLock_();
    mutexSoftLocks_++;
    mutexUnlock_();
}



void Mutex::mutexSoftUnlock()
{
    mutexLock_();
    mutexSoftLocks_--;

    sint temp = mutexSoftLocks_;
    mutexUnlock_();
    eassert(temp >= 0, Exception, "Soft lock count is below zero.");
}



#if TESTING >= TESTLEVEL_IMPORTANT
TEST_BUDDY(mutexLocking)
{
    static Mutex m;
    static sint i = 0;
    class DoesntShare : public seashell::Thread
    {
    public:
        DoesntShare()
        {
            startThread();
        }

        ~DoesntShare()
        {
            stopThread();
        }

        void run()
        {
            LockMutex(m);

	        printf("Lock acquired with i == %i\n", i);
            testAssert(i == 0 || i == 100, "Mutexes did not delay "
              "execution");
            i++;
            for (sint ii = 0; ii < 99; ii++) {
                i++;
                timing::sleepThread(1);
	        }
        }
    };

    {
        m.mutexLock();

        //Assert mutexIsLocked() works
        testAssert(0 != m.mutexIsLocked(), "Did not report lock");

        DoesntShare s1;
        DoesntShare s2;
        timing::sleepThread(1);
        m.mutexUnlock();
    }

    testAssert(i == 200, "Expected i == 400, i = %i", i);
}
END_TEST_BUDDY()
#endif //TESTING

} //seashell
