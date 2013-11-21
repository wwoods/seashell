//Walt Woods
//June 30th, 2007
//MMGR is a memory leak detector and object memory tracker.
//
//
//Usage:
//Simply include this file somewhere between the standard
//library include files and user include files, and any
//memory leaks will be dumped in the file "memleaks.log".
//
//
//Notes:
//Also note that if used in conjunction with the profiler,
//each profiler section will automatically track allocations
//and frees within its code section.

#ifndef MMGR_H_
#define MMGR_H_
#if MMGR

//Files that report errors when used with MMGR if not included before it.
#include <assert.h>
#include <stdlib.h>
#include <new>
#ifdef _WINDOWS
    #include <crtdbg.h>
    #include <cstdlib>
    #include <xmemory>
    #include <xlocale>
    #include <xdebug>
#elif defined(_LINUX)
    #include <string.h>
    #include <memory>
    #include <vector>
#endif
//End MMGR includes

#include "disablemmgrmacros.h"

/**Sets names for the current line.
  */
void mmgr_setNames(char isDelete, const char* file, const sint line, const char* func);

/**Custom functions, named appropriately.
  */
void* mmgr_malloc(size_t size, const char* file, const sint line, const char* func);
void* mmgr_realloc(void* addr, size_t size, const char* file, const sint line, const char* func);
void* mmgr_calloc(size_t count, size_t size, const char* file, const sint line, const char* func);
char* mmgr_strdup(const char* str, const char* file, const sint line, const char* func);
void mmgr_free(void* addr, const char* file, const sint line, const char* func);

/**For instances where you may want to say, overload operator new, it is important that you 
  *first #undef new, but then after defining your operator your should also redefine new to
  *the mmgr version.  This is accomplished by #define new mmgrnew.
  */

#define mmgrnew (mmgr_setNames(0, __FILE__, __LINE__, __FUNCTION__),false) ? 0 : new
#define mmgrdelete (mmgr_setNames(1, __FILE__, __LINE__, __FUNCTION__),false) ? mmgr_setNames(0, 0, 0, 0) : delete
#define mmgrmalloc(t) mmgr_malloc((t), __FILE__, __LINE__, __FUNCTION__)
#define mmgrrealloc(t, sz) mmgr_realloc((t), (sz), __FILE__, __LINE__, __FUNCTION__)
#define mmgrcalloc(cnt, sz) mmgr_calloc((cnt), (sz), __FILE__, __LINE__, __FUNCTION__)
#define mmgrstrdup(t) mmgr_strdup(t, __FILE__, __LINE__, __FUNCTION__)
#define mmgrfree(t) mmgr_free((t), __FILE__, __LINE__, __FUNCTION__)

#define new mmgrnew
#define delete mmgrdelete
#define malloc mmgrmalloc
#define realloc mmgrrealloc
#define calloc mmgrcalloc
#define strdup mmgrstrdup
#define free mmgrfree

#else //MMGR is off
#define MEMUSAGE(t) 
#define mmgrnew new
#define mmgrdelete delete
#define mmgrmalloc malloc
#define mmgrrealloc realloc
#define mmgrcalloc calloc
#define mmgrstrdup strdup
#define mmgrfree free
#endif//MMGR

#endif//MMGR_H_
