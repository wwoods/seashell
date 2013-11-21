//Walt Woods
//August 23rd, 2007
//A thread-specific storage class which allows multiple threads to own private
//copies of a class.

#include <map>

#ifndef THREADPRIVATE_H_
#define THREADPRIVATE_H_

namespace seashell
{

//Thread-local storage class.  Note that, if the class is being deleted, any 
//other members called silently fail.
template<typename T>
class ThreadPrivate : public seashell::Mutex
{
public:
    /**Initializes thread storages
      */
    ThreadPrivate() 
      : destructing(0)
    {
#ifdef _WINDOWS
        tlsIndex = TlsAlloc();
        if (tlsIndex == TLS_OUT_OF_INDEXES)
            ethrow(Exception, "Failure to allocate thread specific key.");
#elif defined(_LINUX)
        sint result = pthread_key_create(&variableKey, 0);
        if (result) {
            ethrow(Exception, "Failure to allocate thread specific key.");
        }
#endif
    }



    /**Destroys all remaining thread storage.
      */
    ~ThreadPrivate()
    {
        LockMutex(*this);
        destructing = 1;

        const sint size = (sint)destroyList.size();
        for (sint i = 0; i < size; i++) {
            delete destroyList[i];
        }

#ifdef _WINDOWS
        TlsFree(tlsIndex);
#elif defined(_LINUX)
        pthread_key_delete(variableKey);
#endif
    }



    /** @return Returns an instance of T that is distinct from thread to 
      *thread.  If the object does not exist, it is created.  If the object
      *cannot be created, an exception is thrown.
      */
    T* get()
    {
        if (destructing)
            return 0;
        
#ifdef _WINDOWS
        T* value = (T*)TlsGetValue(tlsIndex);
#elif defined(_LINUX)
        T* value = (T*)pthread_getspecific(variableKey);
#endif
        if (!value) {
            LockMutex(*this);
#ifdef _WINDOWS
            TlsSetValue(tlsIndex, value = new T());
#elif defined(_LINUX)
            pthread_setspecific(variableKey, value = new T());
#endif
            destroyList.push_back(value);
        }
        return value;
    }



    /**Destroys the current thread's data member.  If the get() function is 
      *called again by this thread, then the object will be re-created.
      */
    void destroy()
    {
        if (destructing)
            return;

#ifdef _WINDOWS
        T* toDestroy = (T*)TlsGetValue(tlsIndex);
#elif defined(_LINUX)
        T* toDestroy = (T*)pthread_getspecific(variableKey);
#endif
        {//Locks the mutex so that the destroy list doesn't change.
            LockMutex(*this);
            const sint size = (sint)destroyList.size();
            for (sint i = 0; i < size; i++) {
                if (destroyList[i] == toDestroy) {
                    destroyList[i] = destroyList[size - 1];
                    destroyList.pop_back();
                    break;
                }
            }
        }

        delete toDestroy;
#ifdef _WINDOWS
        TlsSetValue(tlsIndex, 0);
#elif defined(_LINUX)
        pthread_setspecific(variableKey, 0);
#endif
    }

    void foreach(void (T::* function)(void*), void* param)
    {
        if (destructing)
            return;
        
        SoftLockMutex(*this);

        const sint size = (sint)destroyList.size();
        for (sint i = 0; i < size; i++) {
            (destroyList[i]->*function)(param);
        }
    }

private:
    //List of all created instances.  Makes destroying them easier
    std::vector<T*> destroyList;

    //Are we currently destroying our storage?
    char destructing;

#ifdef _WINDOWS
    DWORD tlsIndex;
#elif defined(_LINUX)
    pthread_key_t variableKey;
#endif
};

} //seashell

#endif//THREADPRIVATE_H_
