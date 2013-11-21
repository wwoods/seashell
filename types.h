
#ifdef bit64
#define BITS 64
#elif defined(bit32)
#define BITS 32
#else
#error Undefined OS bits.  Suggested: define either "bit32" or "bit64" in project settings.
#endif

//Standard, not size sensitive variable types
#if BITS==32
    typedef int sint;
    typedef unsigned int suint;
    typedef float real;
#ifndef _WINDOWS
    typedef unsigned long voidptr;
#else
    typedef unsigned long __w64 voidptr;
#endif

    typedef long long big_sint;
    typedef unsigned long long big_suint;
    typedef double big_real;
#elif BITS==64
#ifdef _WINDOWS
    //Windows names are peculiar
    typedef __int64 sint;
    typedef unsigned __int64 suint;
    typedef double real;
    typedef unsigned __int64 voidptr;
    typedef long long big_sint;
    typedef unsigned long long big_suint;
    typedef long double big_real;
#else
    typedef long sint;
    typedef unsigned long suint;
    typedef double real;
    typedef unsigned long voidptr;

    typedef long long big_sint;
    typedef unsigned long long big_suint;
    typedef long double big_real;
#endif //OS
#endif //BITS

//Standard size sensitive variable types
typedef long long sint64;
typedef int sint32;
typedef short sint16;
typedef char sint8;
typedef unsigned long long suint64;
typedef unsigned int suint32;
typedef unsigned short suint16;
typedef unsigned char suint8;

typedef float real32;
typedef double real64;

//Standard maxes and mins
#define suint8_MAX ((suint8)0xff)
#define sint8_MAX ((sint8)0x7f)
#define suint16_MAX ((suint16)0xffff)
#define sint16_MAX ((sint16)0x7fff)
#define suint32_MAX ((suint32)0xffffffff)
#define sint32_MAX ((sint32)0x7fffffff)

#define real32_MAX ((real)3e38)

#if BITS==32
#define sint_MAX ((sint)0x7fffffff)
#define suint_MAX ((sint)0xffffffff)
#define real_MAX ((real)3e38)
#elif BITS==64
#define sint_MAX ((sint)0x7fffffffffffffff)
#define suint_MAX ((sint)0xffffffffffffffff)
#define real_MAX ((real)3e308)
#endif//BITS

//Platform-specific types:
//  ThreadId - Identifier of a running thread.  
#ifdef _WINDOWS

#include <windows.h>
struct ThreadId
{
    DWORD id;
    HANDLE handle;
    ThreadId() {}
    ThreadId(DWORD d, HANDLE h) : id(d), handle(h) {}
};

#elif defined(_LINUX)

#include <pthread.h>
typedef pthread_t ThreadId;

#endif
