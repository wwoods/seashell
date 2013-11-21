//Walt Woods
//April 20th, 2008

namespace seashell
{

//Recycled pool of objects - any objects not checked in at pool destruction
//will not be freed.  This pool is thread safe.
//
//Constructors and destructors are NOT necessarily called for get or free.
template <typename T>
class RecycledPool
{
public:
    /**A new pool. */
    RecycledPool()
    {
        deletedFlag_ = 0;
    }



    /**Frees all the objects that have been checked in. */
    ~RecycledPool()
    {
        //Set the deleted flag
        deletedFlag_ = 1;

        //Free all of the listed objects
        T* obj;
        while (obj = freeObjects_.pop_back()) {
            delete obj;
        }
    }

    /**Allocates a new resource structure.  Doesn't initialize it or any 
      *such thing. */
    T* getNewStruct()
    {
        T* ret = 0;

        //Make sure we aren't deleted
        eassert(!deletedFlag_, Exception, "RecycledPool allocation after "
          "deletion.");
        LockMutex(mLock_);

        //Check if there's one in the stash
        ret = freeObjects_.pop_back();
        if (ret)
            return ret;
        return new T();
    }

    /**Frees a previously retrieved structure.  */
    void freeStruct(T* obj)
    {
        if (deletedFlag_) {
            //We've been deleted; free the object statically.
            delete obj;
            return;
        }

        LockMutex(mLock_);

        //Register the structure for the object list
        freeObjects_.push_back(obj);
    }

private:
    //Mutex for locking the pool
    seashell::Mutex mLock_;

    //Pool of resource pointers.  Most basic vector ever
    class FreeObjects {
    public:
        FreeObjects() :
          size_(0), maxsize_(10)
        {
            pool_ = (T**)malloc(sizeof(T*) * maxsize_);
        }

        ~FreeObjects()
        {
            free(pool_);
        }

        void push_back(T* obj)
        {
            if (size_ == maxsize_)
                double_();

            pool_[size_++] = obj;
        }

        T* pop_back()
        {
            if (size_ > 0)
                return pool_[--size_];
            return 0;
        }

        void double_()
        {
            T** temp;
            maxsize_ *= 2;

            temp = (T**)malloc(sizeof(T*) * maxsize_);
            memcpy(temp, pool_, sizeof(T*) * size_);
            free(pool_);

            pool_ = temp;
        }

        suint size_;
        suint maxsize_;
        T** pool_;
    } freeObjects_;

    //Static flag for signalling that this type of recycled pool has been 
    //deleted.
    static char deletedFlag_;
};

} //seashell
