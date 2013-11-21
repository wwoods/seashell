//Walt Woods
//June 30th, 2007

//This file should ALWAYS be compiled with full compiler optimizations (that's
//not to say that FAST need be defined.

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "seashell.h"
#include "profiler_timinginfo.h"

#if MMGR

#include "disablemmgrmacros.h"

void mmgr_unsetNames();

namespace mmgr
{
//Settings
//------------
//memleakFile is the file to output all memory leaks to at the end of program execution.
char memleakFile[] = "memleaks.log";

//Our memory related assertion types.  These cite the last known file and line number
//instead of the line number on which the assertion is declared.
#define massert(b, ...) if (!(b)) { throw Exception(mmgr::priorerror.file, \
  mmgr::priorerror.line, mmgr::priorerror.func, __VA_ARGS__); }

/**Is the manager initialized?
  */
char initialized = 0;

/**defAllocSig is the signature given to all AllocReferences as their first four bytes.
  */
sint defAllocSig = 0x80ff80ff;

//The enum memtype stores the type of allocation, and therefore,
//also the type of deletion that should accompany it.
const char* ALLOC_NEW = "new/delete";
const char* ALLOC_NEW_ARRAY = "new[]/delete[]";
const char* ALLOC_MALLOC = "malloc/free";

/**AllocTimeReference points to a time - in the code - at which an event occurs.
  */
struct AllocTimeReference
{
    const char* file;
    sint line;
    const char* func;
};

/**AllocReference is a doubly linked list that tracks all outstanding memory
  *allocations.  At the end of the application, the contents of this list are dumped
  *to the file defined in memleakFile.
  */
struct AllocReference
{	
    //This flag tells the application that this is a valid AllocReference
    //Set at the beginning to prioritize its placement at the front, in order 
    //to maximize the chance that it will become corrupt as opposed to 
    //anything else.
    sint allocSig;
    
    //The requested size of this allocation
    size_t size;
    
    //The method of allocation (Ensures matching new and delete or malloc and free)
    const char* alloctype;

#if PROFILE
    //The profiler stack at time of creation
    void* profilerFingerprint;
#endif
    
    //Information on when this allocation was created
    AllocTimeReference creation;

    //The next allocation reference
    AllocReference* pNext;
    
    //The previous allocation reference
    AllocReference* pPrev;
    
    //For allocations that did not get mmgr_setNames() called, the best we can do
    //is triangulate a position by the subsequent and immediately following 
    //allocations or deallocations that ARE known.
    //
    //OBSOLETE.  Now, Profiler allows a stack trace() function that will provide
    //contextual information about allocations.
};

//The values that fill the checks are actually references to the relevant memory pointer.
//must be at least 1; number of pointers on either side of each allocation.
const size_t checks = 16; 
const size_t checktypesize = sizeof(AllocReference*);
//Size of one side of checks
const size_t checksize = checks * checktypesize;

//As described in Paul Nettles' code, C++ will call operator new as:
//mmgr_setNames(), then operator new, then the constructor.
//This is fine.  However, operator delete will be called:
//mmgr_setNames(), destructor, operator delete.
//This is a problem if destructor calls a delete operator.  As such, we always keep the first
//mmgr_setNames() call, and label all subsequent deletions as "owned" deletions.
//sint deleteDepth = 0;
//
//Also, this implies that we will always know the location of a new operator, if it is 
//called under the domain of our "new" macro, but we will not always know the 
//same of the delete.  However, to fix this we can simply store the line number with
//the new operator, and check if its value is 0 in the destructor.

//lastknown stores the last known location settings from mmgr_setNames.
AllocTimeReference lastknown = { 0 };

//priorerror is similar to lastknown, but it is never unset.
AllocTimeReference priorerror = { 0 };



//This class manages the AllocReference chain and protects it from multithread
//damages.
static class AllocReferenceManager
{
public:
    /**Constructor; initializes object.
      */
    AllocReferenceManager()
    {
        initialize();
    }



    /**Initializes the chain.
      */
    void initialize()
    {
        _root = (AllocReference*)malloc(sizeof(AllocReference));
        _root->allocSig = defAllocSig;
        _root->pNext = _root;
        _root->pPrev = _root;
        _root->creation.line = 1; //make the root appear valid for previous allocs needing names.

        _deallocating_all = 0;
    }



    /**Once a new AllocReference has been allocated, fix its pointers into the 
      *chain.
      *Also updates all memory trackers
      *Does all protected allocating
      */
    void allocFix(AllocReference* ar)
    {
        LockMutex(_mutex);

        ar->pPrev = _root->pPrev;
        ar->pNext = _root;
        _root->pPrev->pNext = ar;
        _root->pPrev = ar;
        ar->allocSig = defAllocSig;
    }



    /**Prior to freeing an AllocReference, the appropriate pointers must be 
      *replaced.
      *Does all protected releasing
      */
    void allocUnfix(AllocReference* ar)
    {
        LockMutex(_mutex);

        ar->pNext->pPrev = ar->pPrev;
        ar->pPrev->pNext = ar->pNext;
        ar->allocSig = 0;
    }



    /**Alerts the manager that statics are being freed.
      */
    void staticDestruction()
    {
        _deallocating_all = 1;
        printMemleaks();
    }



    /**Prints a list of memory leaks.
      */
    void printMemleaks()
    {
        if (!_deallocating_all)
            return;
        
        FILE* f = fopen("memleaks.log", "wt");
        eassert(f, Exception, "Unable to open memleak file for output.");

        static int numPrints = 0;
        numPrints++;
        
        fprintf(f, "Memleaks log report print #%i\n-------------------\n", numPrints);
        AllocReference* ar = _root->pNext;
        if (ar == _root)
            fprintf(f, "No memory leaks detected.\n");
        else {
            while (ar != _root) {
#if PROFILE
                fprintf(f, "Profiler stack trace:\n");
                profiler::printStackTrace(f, ar->profilerFingerprint);
#endif
                if (ar->creation.line > 0) {
                    fprintf(f, "%s(%i)\nFunction '%s', size %i\n"
                      "-------------------\n", ar->creation.file, 
                      ar->creation.line, ar->creation.func, ar->size);
                }
                else {
#if !PROFILE
                    fprintf(f, "Recommended turn on profiling to find "
                      "leak.\n");
#endif
                    fprintf(f, "Exact location unknown\n  size %i\n"
                      "-------------------\n", ar->size);
                }
                ar = ar->pNext;
            }
        }
            
        fclose(f);
    }

    //Bypass global operator new
    void* operator new(std::size_t size)
    {
        return malloc(size);
    }

    void operator delete(void* mem)
    {
        free(mem);
    }
    
private:
    /**The root AllocReference.
      */
    AllocReference* _root;

    /**Static deallocation time.
      */
    char _deallocating_all;

    /**Locking mutex.
      */
    seashell::Mutex _mutex;
} *_allocReferences;



AllocReferenceManager* allocReferences()
{
    if (!_allocReferences) {
        //Necessary to lock first, in case of double-construction race 
        //condition.
        _allocReferences = new AllocReferenceManager();
    }
    return _allocReferences;
}



/**The timekeeper class which automatically logs all leaks at the end of 
  *execution.
  */
static class mmgr_timekeeper
{public:
    mmgr_timekeeper() {}
    ~mmgr_timekeeper() { allocReferences()->staticDestruction(); }
} mmgrtime;



/**init() sets up root and various other global variables pertaining to the manager.
  */
void initialize()
{
    if (!initialized) {
        initialized = 1;

        allocReferences();
    }
}

void* allocator(const char* type, size_t size)
{	
    try {
        initialize();

        //Solaris compilant alignment
        static const sint bytes = BITS / 8;
        if (size % bytes != 0)
            size += bytes - (size % bytes);

        const size_t realSize = sizeof(AllocReference) + size + 2 * checksize;
        char* ret = (char*)malloc(realSize);
        massert(ret, "Allocation failed - out of memory? (Attempted %i bytes)",
          (sint)size);
        
        //First chunk of memory is the allocation reference.
        AllocReference* ar = (AllocReference*)ret;
        ret += sizeof(AllocReference);

#if PROFILE
        ar->profilerFingerprint = profiler::getStackFingerprint();
#endif
    
        ar->size = size;
        ar->alloctype = type;
        if (lastknown.line > 0)	{
            ar->creation = lastknown;
        }
        else {
            ar->creation.line = 0;
        }

        allocReferences()->allocFix(ar);
        
        char* temp = ret;

        //Wipe before and after the data with pointers to the AllocReference
        for (size_t i = 0; i < checks; i++) {
            *(AllocReference**)temp = ar;
            temp += checktypesize;
        }
        temp += size;
        for (size_t i = 0; i < checks; i++) {
            *(AllocReference**)temp = ar;
            temp += checktypesize;
        }
        
#if PROFILE
        //Update usage information
        profiler::TimingInfo* timing = 
          (profiler::TimingInfo*)ar->profilerFingerprint;
        if (timing)
            timing->result.allocations += (big_suint)size;
#endif

        //After each new, it is ok to unset the names.  We know we have all of
        //the information we need from them.
        mmgr_unsetNames();

        ret = ret + checksize;
        return (void*)ret;
    }
    catch (const Exception& e) {
        elog(e);
    }
    catch (...) {
    }
    throw std::bad_alloc();
}

void deallocator(const char* type, void* addr)
{
    if (addr == 0) {
        return;
    }

    char* const realaddr = (char*)addr - checksize - sizeof(AllocReference);
    AllocReference* ar = (AllocReference*)realaddr;
    try {
        initialize();

        char* temp = realaddr;
        massert(ar->allocSig == defAllocSig, "Pre-allocation memory corrupted");
            
        //---------------------------------
        //Update allocation tables
        //---------------------------------
        if (ar->creation.line > 0) {
            mmgr_unsetNames();
        }

        //Update usage information
#if PROFILE
        profiler::TimingInfo* current = 
          (profiler::TimingInfo*)profiler::getStackFingerprint();
        if (current) {
            current->result.deallocations += (big_suint)ar->size;
        }
#endif

        //---------------------------------
        //Check for further memory corruption before freeing.
        //---------------------------------

        //Test for allocator type mismatches
        if (ar->alloctype != type) {
            static char errstr[512];
            StringCchPrintf(errstr, sizeof(errstr),
              "Mismatching allocator and deallocator: allocated like %s, "
              "deallocated like %s", ar->alloctype, type);
            massert(0, errstr);
            //In RARE case this could mean pre-allocation corruption
        }

        //Test for memory corruption
        temp += sizeof(AllocReference);
        for (size_t i = 0; i < checks; i++) {
            massert(ar == *(AllocReference**)temp, 
              "Pre-allocation memory corrupted");
            temp += checktypesize;
        }
        temp += ar->size;
        for (size_t i = 0; i < checks; i++) {
            massert(ar == *(AllocReference**)temp, 
              "Post-allocation memory corrupted");
            temp += checktypesize;
        }
    }
    catch (const Exception& e) {
        elog(e);
    }
    catch (...) {
        elog(makeException("Unknown exception thrown in deallocator."));
    }

    allocReferences()->allocUnfix(ar);
    free(realaddr);
    allocReferences()->printMemleaks();
}

} //mmgr

//File name of impending
void mmgr_setNames(char isDelete, const char* file, const sint line, const char* func)
{
    mmgr::lastknown.file = file;
    mmgr::lastknown.line = line;
    mmgr::lastknown.func = func;

    mmgr::priorerror.file = file;
    mmgr::priorerror.line = line;
    mmgr::priorerror.func = func;
}

void mmgr_unsetNames()
{
    //Line is the only true marker.
    mmgr::lastknown.line = 0;
}

void* operator new(size_t size)
{
    return mmgr::allocator(mmgr::ALLOC_NEW, size);
}

void* operator new[](size_t size)
{
    return mmgr::allocator(mmgr::ALLOC_NEW_ARRAY, size);
}

//Form of new used by some microsoft compilers
void	*operator new(size_t reportedSize, const char *sourceFile, int sourceLine)
{
    return mmgr::allocator(mmgr::ALLOC_NEW, reportedSize);
}

//Form of new used by some microsoft compilers
void	*operator new[](size_t reportedSize, const char *sourceFile, int sourceLine)
{
    return mmgr::allocator(mmgr::ALLOC_NEW_ARRAY, reportedSize);
}

void operator delete(void* addr)
{
    mmgr::deallocator(mmgr::ALLOC_NEW, addr);
}

void operator delete[](void* addr)
{
    mmgr::deallocator(mmgr::ALLOC_NEW_ARRAY, addr);
}

void* mmgr_malloc(size_t size, const char* file, const sint line, const char* func)
{
    mmgr_setNames(0, file, line, func);
    void* ret;
    try {
        ret = mmgr::allocator(mmgr::ALLOC_MALLOC, size);
        return ret;
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

void* mmgr_realloc(void* addr, size_t size, const char* file, const sint line, const char* func)
{
    mmgr_setNames(0, file, line, func);
    void* ret;
    try {
        mmgr::AllocReference* ar = (mmgr::AllocReference*)
          ((char*)addr - mmgr::checksize - sizeof(mmgr::AllocReference));
        ret = mmgr::allocator(mmgr::ALLOC_MALLOC, size);
        memcpy(ret, addr, ar->size);

        mmgr_setNames(1, file, line, func);
        mmgr::deallocator(mmgr::ALLOC_MALLOC, addr);
        return ret;
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

void* mmgr_calloc(size_t count, size_t size, const char* file, const sint line, const char* func)
{
    mmgr_setNames(0, file, line, func);
    void* ret;
    try {
        ret = mmgr::allocator(mmgr::ALLOC_MALLOC, count * size);
        memset(ret, 0, count * size);
        return ret;
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

char* mmgr_strdup(const char* str, const char* file, const sint line, const char* func)
{
    mmgr_setNames(0, file, line, func);
    char* ret;
    try {
        ret = (char*)mmgr::allocator(mmgr::ALLOC_MALLOC, strlen(str) + 1);
        StringCchCopy(ret, strlen(str) + 1, str);
        return ret;
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

void mmgr_free(void* addr, const char* file, const sint line, const char* func)
{
    mmgr_setNames(1, file, line, func);
    mmgr::deallocator(mmgr::ALLOC_MALLOC, addr);
    //mmgr_unsetNames(); Done automatically in deallocator
}

#endif//MMGR
