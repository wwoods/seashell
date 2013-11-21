//Walt Woods
//August 8th, 2007
//Portable mutex class for stalling thread execution.

#ifndef THREAD_MUTEX_H_
#define THREAD_MUTEX_H_

#ifdef _WINDOWS
#include <windows.h>
#elif defined(_LINUX)
#include <pthread.h>
#endif

namespace seashell
{

//Derivable class Mutex.  Any class derived from Mutex will obtain the 
//functions mutexLock and mutexUnlock, and as such secure a method for thread
//synchronization.  For local-scope locks, use the macro LockMutex().
//
//This class also features soft locks, which are non-exclusive.  Soft locks
//imply that a function reads the data, and will be corrupted if the data is
//changed, but that the function itself will not change the data.  The macro
//SoftLockMutex() will soft lock the mutex.
//
//There is no way to graduate a soft lock into a hard lock.  The reason for 
//this comes about when multiple soft locks want to graduate to a hard lock -
//a deadlock results.
class Mutex
{
public:
    /**Creates and initializes a locking mechanism for this object.
      */
    Mutex();

    /**Destroys the Mutex object.  Invalidates the locking mechanism.
      */
    virtual ~Mutex();

    /** @return Returns non-zero if the mutex is locked in any way. */
    char mutexIsLocked();

    /**Locks the mutex.  Locks do not allow stacking - only one lock may be on
      *a mutex at a time.
      *This function blocks until the mutex is acquired.
      */
    void mutexLock();

    /**Locks the mutex.
      *This function attempts to acquire the mutex once, and returns whether
      *it was successful or not.
      * @return Returns non-zero if the calling thread now has ownership.  
      *Otherwise, the return value is 0 and the application should not attempt
      *to access the data.
      */
    sint mutexTryLock();

    /**Unlocks the mutex.
      */
    void mutexUnlock();

    /**Soft locks the mutex.
      */
    void mutexSoftLock();

    /**Unlocks a soft lock on the mutex.
      */
    void mutexSoftUnlock();

private:
    //Is the mutex hard locked?
    sint mutexLocks_;

    //Number of active soft locks
    volatile sint mutexSoftLocks_;

    //Does the mutex want a hard lock?  Prohibits soft locking when active
    volatile char mutexWantsHardlock_;

    //Internal locking functions.  Do not respect soft locks.
    void mutexLock_();
    sint mutexTryLock_();
    void mutexUnlock_();

#ifdef _WINDOWS
    CRITICAL_SECTION mutex_;
#elif defined(_LINUX)
    pthread_mutex_t mutex_;
#endif //Operating systems
};



//Scope-lock of mutex.  
#define MutexLockerJoin(a, b) a##b
#define LockMutex(m) seashell::MutexLocker MutexLockerJoin(locker, __LINE__)(m)
#define SoftLockMutex(m) seashell::SoftMutexLocker MutexLockerJoin(locker, \
  __LINE__)(m)
//Scope locker.  Blocks until mutex is acquired.
class MutexLocker
{
public:
    /**Locks the specified mutex. 
      */
    MutexLocker(Mutex& lock);

    /**Releases the lock. 
      */
    ~MutexLocker();

private:
    Mutex* lock_;
};

class SoftMutexLocker
{
public:
    /**Soft locks the specified mutex. 
      */
    SoftMutexLocker(Mutex& lock);

    /**Releases the lock. 
      */
    ~SoftMutexLocker();

private:
    Mutex* lock_;
};

} //seashell

#endif//THREAD_MUTEX_H_
