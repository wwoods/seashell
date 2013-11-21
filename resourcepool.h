//Walt Woods
//April 18th, 2008

#ifndef SEASHELL_RESOURCE_POOL_H_
#define SEASHELL_RESOURCE_POOL_H_

namespace seashell
{

//A linear list of resources with a unique identifier for each.  Thread safe.
template<typename T>
class ResourcePool
{
public:
    /**Clears the pool tables. */
    void clear()
    {
        LockMutex(mLock_);

        //Clear lists
        resourceList_.clear();
        freeList_.clear();
    }

    /**Allocates an id for the specified object and stores it in the table. 
      * @return Returns the uid allocated for the object.  */
    suint assign(T* obj)
    {
        suint uid;
        LockMutex(mLock_);

        //Goto reference
        while (1) {
            //Is there a free slot?
            if (freeList_.size() != 0) {
                //Allocate the back free slot to this new object.
                uid = freeList_.back();
                freeList_.pop_back();
                if (resourceList_[uid] != 0) {
                    //Already taken; specific id assignment?  Look for next 
                    //slot (we already removed the freeList_ entry)
                    continue;
                }
                resourceList_[uid] = obj;
            }
            else {
                //Allocate a new slot at the back of the resource list
                uid = (suint)resourceList_.size();
                resourceList_.push_back(obj);
            }
            break;
        }

        //Return identifier
        return uid;
    }

    /**Allocates the specified id for the specified object and stores it in 
      *the table. 
      * @return Returns the uid parameter passed.  
      * @throw Exception Thrown when the uid requested is already taken. */
    suint assign(suint uid, T* obj)
    {
        //Over allocation for uids out of range, to avoid constantly resizing
        //the array.
        static const suint OVER_ALLOCATE = 100;

        suint size;
        LockMutex(mLock_);

        //Resize the list if necessary
        size = (suint)resourceList_.size();
        if (uid >= size) {
            //Set the list to one larger than the uid
            resourceList_.resize(uid + 1 + OVER_ALLOCATE);

            //Make note of new free spots
            for (suint i = size; i < uid; i++) {
                freeList_.push_back(i);
                resourceList_[i] = 0;
            }

            //And the other half of the storage
            for (suint i = uid + 1; i < uid + 1 + OVER_ALLOCATE; i++) {
                freeList_.push_back(i);
                resourceList_[i] = 0;
            }

            //Zero uid spot as well
            resourceList_[uid] = 0;
        }
        else {
            //Check if the spot requested is free
            if (resourceList_[uid] != 0) {
                ethrow(Exception, "Pool attempted to assign specific id to "
                  "new object, but the id is already in use.");
            }
        }

        //Register the object in the uid slot; we don't care if the freeList_
        //is slightly erroneous.  It's not worth checking all of its entries.
        resourceList_[uid] = obj;

        //Return identifier
        return uid;
    }

    /**Frees a previously allocated object based on its id.  This DOES NOT 
      *delete the object; it merely frees its id slot. */
    void release(suint uid)
    {
        LockMutex(mLock_);

        //Add the slot to the free list
        freeList_.push_back(uid);

        //Set the resource pointer to null
        resourceList_[uid] = 0;
    }

    /**Volatile.  The returned value may be deleted before it is used if the 
      *application is not developed properly.
      * @return Returns an object for the given identifier.  Can return null if
      *the object doesn't exist or the uid requested doesn't exist. */
    T* get(suint uid)
    {
        if (uid >= (suint)resourceList_.size())
            return 0;

        SoftLockMutex(mLock_);
        T* result = resourceList_[uid];
        return result;
    }

    /**Calls the specified function for every object in the list.  Be warned;
      *as with get(), the objects passed to the function can be volatile if 
      *the application is not well designed. */
    void callForEach(void (*func)(T*, void*), void* param)
    {
        SoftLockMutex(mLock_);
        T* obj;

        //Save a few cycles by caching the size
        const suint max = (suint)resourceList_.size();
        for (suint i = 0; i < max; i++) {
            //Store object in advance so that it cannot be erased between 
            //checking for null and passing it to the function.
            obj = resourceList_[i];

            if (obj) {
                func(obj, param);
            }
        }
    }

private:
    //Mutex to control thread lockin    g
    Mutex mLock_;

    //List of currently free identifiers
    std::vector<suint> freeList_;

    //List of objects
    std::vector<T*> resourceList_;
};

} //seashell

#endif//SEASHELL_RESOURCE_POOL_H_
