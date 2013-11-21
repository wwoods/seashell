//Walt Woods
//August 5th, 2007
//Various pointers used for freeing and deleting objects regardless of
//execution flow.  The main difference between these and auto_ptr is 
//that these are implemented as 100% transparent; auto_ptr on the other hand
//will not allow you to pass the raw pointer without specifying it.

//Trying to assign one of these pointers to another pointer of their type will
//nullify the original pointer.

//These look like auto_ptr, but I like distinguishment between array and 
//non-array versions.

//IMPROPER USES:
//Function parameters - Will always delete the object inside of the function,
//  without trickery.  Bad idea.

//PROPER USES:
//In-scope variable management - What these are designed for.  Delete the 
//  object when it goes out of scope.
//Function return values - Ensures that the return value will get deleted;
//  makes apparent that the return value is allocated memory.  HOWEVER,
//  the function retrieving the return value MUST BE SURE to assign it to 
//  another ptr pointer class, or else it will be deleted immediately.

//These classes should be used heavily in regions where:
//A) A pointer may not necessarily be assigned an object.
//B) If it is, the object must be deallocated even if exceptions are thrown.

//del_ptr<TYPE> - Pointer to TYPE*.  If its value is non-zero at the time its
//                scope is exited, the object is delete'd.
//del_array_ptr<TYPE> - Pointer to TYPE*.  If its value is non-zero at the time its
//                      scope is exited, the object is delete[]'d.
//free_ptr<TYPE> - Pointer to TYPE*.  If its value is non-zero at the time its
//                 scope is exited, the object is free()'d.
//handle_to<TYPE> - Controlled handle to TYPE*.  Performs reference counting. 
//                  Can emulate copy on write with unique().
//handle_to_array<TYPE> - Array version (delete[]) of handle_to.



//Once created, can be used like a normal pointer.  If pointer
//is non-null at destruction, the object pointed to is delete'd.
template<typename T>
class del_ptr
{
public:
    /**Plain constructor.  Initializes pointer to 0.
      */
    del_ptr()
        : _pointer(0)
    {}

    /**Transfers object ownership from another del_ptr.  Useful for return 
      *values.
      */
    del_ptr(const del_ptr& ptr)
        : _pointer(ptr._pointer)
    {
        const_cast<del_ptr&>(ptr)._pointer = 0;
    }

    /**Constructs this pointer off of a plain pointer
      *to this object type.
      * @param _ptr Pointer to set this objects' pointer.
      */
    del_ptr(T* ptr)
        : _pointer(ptr)
    {}

    /**If pointer is non-null, the object pointed to is
      *free()'d.
      */
    ~del_ptr()
    { delete _pointer; }

    /**Updates this object's pointer.
      * @param _ptr New pointer value.
      * @return Returns the new pointer.
      */
    T* operator=(T* ptr)
    {
        delete _pointer;
        _pointer = ptr;
        return ptr;
    }

    /**Updates this object's pointer based on another ptr class's
      *pointer.  Unsets the other pointer.
      * @param _ptr Another del_ptr.
      * @return Returns the new pointer value.
      */
    T* operator=(const del_ptr<T>& ptr)
    {
        delete _pointer;
        _pointer = ptr._pointer;
        const_cast<del_ptr&>(ptr)._pointer = 0;
        return _pointer;
    }

    /**Releases the pointer to an unmanaged state. */
    T* release()
    {
        T* temp = _pointer;
        _pointer = 0;
        return temp;
    }

    /**Override -> to allow this pointer to be used as a normal pointer.
      */
    T* operator->() const
    {
        return _pointer;
    }

    /**To aid transparency, allow implicit conversion to type T*.
      */
    operator T*() const
    {
        return _pointer;
    }

private:
    /**The pointer we are maintaining.
      */
    T* _pointer;
};



//Once created, can be used like a normal pointer.  If pointer
//is non-null at destruction, the object pointed to is delete'd.
template<typename T>
class del_array_ptr
{
public:
    /**Plain constructor.  Initializes pointer to 0.
      */
    del_array_ptr()
        : _pointer(0)
    {}

    /**Transfers object ownership from another del_ptr.  Useful for return 
      *values.
      */
    del_array_ptr(const del_array_ptr& ptr)
        : _pointer(ptr._pointer)
    {
        const_cast<del_array_ptr&>(ptr)._pointer = 0;
    }

    /**Constructs this pointer off of a plain pointer
      *to this object type.  Also works correctly for 
      *other free_ptr's of the same time.
      * @param _ptr Pointer to set this objects' pointer.
      */
    del_array_ptr(T* ptr)
        : _pointer(ptr)
    {}

    /**If pointer is non-null, the object pointed to is
      *free()'d.
      */
    ~del_array_ptr()
    { delete[] _pointer; }

    /**Updates this object's pointer.
      * @param _ptr New pointer value.
      * @return Returns the new pointer.
      */
    T* operator=(T* ptr)
    {
        delete[] _pointer;
        _pointer = ptr;
        return ptr;
    }

    /**Updates this object's pointer based on another ptr class's
      *pointer.  Unsets the other pointer.
      * @param _ptr Another del_array_ptr.
      * @return Returns the new pointer value.
      */
    T* operator=(const del_array_ptr<T>& ptr)
    {
        delete[] _pointer;
        _pointer = ptr._pointer;
        const_cast<del_array_ptr&>(ptr)._pointer = 0;
        return _pointer;
    }

    /**Releases the pointer to an unmanaged state. */
    T* release()
    {
        T* temp = _pointer;
        _pointer = 0;
        return temp;
    }

    /**Override -> to allow this pointer to be used as a normal pointer.
      */
    T* operator->() const
    {
        return _pointer;
    }

    /**To aid transparency, allow implicit conversion to type T*.
      */
    operator T*() const
    {
        return _pointer;
    }

private:
    /**The pointer we are maintaining.
      */
    T* _pointer;
};



//Once created, can be used like a normal pointer.  If pointer
//is non-null at destruction, the object pointed to is free()'d.
template<typename T>
class free_ptr
{
public:
    /**Plain constructor.  Initializes pointer to 0.
      */
    free_ptr()
        : _pointer(0)
    {}

    /**Transfers object ownership from another del_ptr.  Useful for return 
      *values.
      */
    free_ptr(const free_ptr& ptr)
        : _pointer(ptr._pointer)
    {
        const_cast<free_ptr&>(ptr)._pointer = 0;
    }

    /**Constructs this pointer off of a plain pointer
      *to this object type.  Also works correctly for 
      *other free_ptr's of the same time.
      * @param _ptr Pointer to set this objects' pointer.
      */
    free_ptr(T* ptr)
        : _pointer(ptr)
    {}

    /**If pointer is non-null, the object pointed to is
      *free()'d.
      */
    ~free_ptr()
    { free(_pointer); }

    /**Updates this object's pointer.
      * @param _ptr New pointer value.
      * @return Returns the new pointer.
      */
    T* operator=(T* ptr)
    {
        free(_pointer);
        _pointer = ptr;
        return ptr;
    }

    /**Updates this object's pointer based on another ptr class's
      *pointer.  Unsets the other pointer.
      * @param _ptr Another free_ptr.
      * @return Returns the new pointer value.
      */
    T* operator=(const free_ptr<T>& ptr)
    {
        free(_pointer);
        _pointer = ptr._pointer;
        const_cast<free_ptr&>(ptr)._pointer = 0;
        return _pointer;
    }

    /**Releases the pointer to an unmanaged state. */
    T* release()
    {
        T* temp = _pointer;
        _pointer = 0;
        return temp;
    }

    /**Override -> to allow this pointer to be used as a normal pointer.
      */
    T* operator->() const
    {
        return _pointer;
    }

    /**To aid transparency, allow implicit conversion to type T*.
      */
    operator T*() const
    {
        return _pointer;
    }

private:
    /**The pointer we are maintaining.
      */
    T* _pointer;
};

//Handle to an object.  Also has member unique(), which emulates copy 
//on write functionality.
//
//Example usage:
//handle_to<int> i; //creates a new integer and stores it in the handle
//*i = 5;           //Casts the handle as an object (returns Object&)
//
// @param T Class to present a handle to
// @param cache If non-zero, then cache this number of objects in RAM for 
//re-use.  The obvious downside to this is the mutex locking required to 
//safely request an object from the cache.
// @param resetCached Called when an object is allocated and caching is 
//enabled.
// @param clearCached Called when an object is released and caching is 
//enabled.
template<typename T, 
  suint cache = 0,
  void (*resetCached)(T*) = 0,
  void (*clearCached)(T*) = 0>
class handle_to
{
    //Structure for handling objects
    struct Object {
        T t;
        suint refcount;

        Object() {}
        Object(const T& obj) : t(obj) {}
        ~Object() {}
    };

    /**Returns a new Object. */
    static Object* allocateObject()
    {
        Object* ret = new Object();

        //If we're doing caching, call resetCached
        if (resetCached)
            resetCached(&ret->t);

        return ret;
    }

    /**Returns a new Object.  Copies another Object's values.  */
    static Object* allocateObject(const T& other)
    {
        Object* ret = new Object(other);

        //If we're doing caching, call resetCached
        if (resetCached)
            resetCached(&ret->t);

        return ret;
    }

    /**Deallocates an Object. */
    static void deallocateObject(Object* obj)
    {
        //If we're doing caching, call clearCached
        if (clearCached)
            clearCached(&obj->t);

        delete obj;
    }

public:
    /**Constructs the handle with an object.  */
    handle_to()
    {
        //Allocate pointer
        pointer_ = allocateObject();

        //Initialize refcount
        pointer_->refcount = 1;
    }

    /**Creates the object by duplicating another.  Creates a new handle for the
      *duplicate.  */
    handle_to(const T& other)
    {
        //Allocate pointer
        pointer_ = allocateObject(other);

        //Initialize refcount
        pointer_->refcount = 1;
    }

    /**Duplicates a handle (don't want const ref; we change their refcount) */
    handle_to(handle_to& ptr)
        : pointer_(ptr.pointer_)
    {
        eassert(pointer_, Exception, "Handles must not be null.");
        pointer_->refcount++;
    }

    /**Frees the object if *refcount_ == 0 */
    ~handle_to()
    {
        release_();
    }

    /**Updates this object's pointer based on another ptr class's
      *pointer.  Unsets our original pointer.  Increments the new pointer.
      * @param _ptr Another del_ptr.
      * @return Returns the new pointer value. */
    handle_to<T>& operator=(handle_to<T>& ptr)
    {
        //Unset our pointer
        release_();

        //Adopt the new pointer, increment its refcount
        pointer_ = ptr.pointer_;
        pointer_->refcount++;
        return *this;
    }

    /**Override -> to allow this pointer to be used as a normal pointer. */
    T* operator->()
    {
        return &pointer_->t;
    }

    /**Override -> to allow this pointer to be used as a normal pointer. */
    const T* operator->() const
    {
        return &pointer_->t;
    }

    /**Retrieves the pointer. */
    T* get()
    {
        return &pointer_->t;
    }

    /**Retrieves the pointer. */
    const T* get() const
    {
        return &pointer_->t;
    }

    /**To aid transparency, allow pointer dereferences. */
    T& operator*()
    {
        return pointer_->t;
    }

    /**To aid transparency, allow pointer dereferences. */
    const T& operator*() const
    {
        return pointer_->t;
    }

    /**Copies the object if its reference count is not unique. 
      * @return Returns the newly formed object, or the old if it was unique */
    T* getUniqueCopy()
    {
        copy_();
        return &pointer_->t;
    }


private:
    /**The pointer we are maintaining. */
    Object* pointer_;

    /**Release a reference. */
    void release_()
    {
        eassert(pointer_, Exception, "Handles must have a pointer");

        suint& refcount = pointer_->refcount;
        refcount--; 
        if (refcount == 0) {
            //Delete object
            deallocateObject(pointer_);
        }

        //Unset our pointer
        pointer_ = 0;
    }

    /**Copy the object if refcount_ != 1 */
    void copy_()
    {
        eassert(pointer_, Exception, "Handles must have a pointer");

        if (pointer_->refcount != 1) {
            Object* temp = new Object(pointer_->t);
            release_();

            pointer_ = temp;
            temp->refcount = 1;
        }
    }
};
