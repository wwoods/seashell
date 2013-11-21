//Walt Woods
//Started June 27th, 2007
//Collection of utility classes to enhance production of all kinds of software.
//Everything in seashell MUST be portable.  These are foundations.

#ifndef SEASHELL_H_
#define SEASHELL_H_

//OS Settings
#ifdef _WINDOWS
    #ifdef _MSC_VER
        #pragma warning(disable:4996)
    #endif
#elif defined(_LINUX)
    typedef std::size_t size_t;
#else
    #error Undefined/unsupported operator system.  Valid: _WINDOWS or _LINUX.
#endif

//Output profiler / exceptions to console?
//#define CONSOLE_OUTPUT

//Test level defines.  These are primarily biased based on the time it takes
//to complete a test.  All tests should be placed in 
//#if TESTING >= TESTLEVEL_XXXX
//blocks.
#define TESTLEVEL_NONE 0
#define TESTLEVEL_CORE 1
#define TESTLEVEL_IMPORTANT 2
#define TESTLEVEL_THOROUGH 3

//Handle defines
#ifdef FAST
    #define PROFILE 0
    #define MMGR 0
    #define TESTING TESTLEVEL_CORE
    #define ASSERTIONS 0
#else //!FAST - Debug mode
    #define PROFILE 1
    #define MMGR 1
    #define TESTING TESTLEVEL_IMPORTANT
    #define ASSERTIONS 1
#endif

#include <string>
#include <vector>

//------------------------------
//    Core Functionality
//------------------------------
//Standard variable types.
#include "types.h"

//Standard defines (PI, EPSILON, etc)
#include "defines.h"

//Include default exceptions
#include "exception.h"

//Include the memory manager
#include "mmgr.h"

//Include the profiler
#include "profiler.h"

//Include the test buddy system
#include "testbuddy.h"

//------------------------------
//    Utility Functionality
//------------------------------
//Timing functions
#include "timing.h"

//System information functions
#include "systeminfo.h"

//Clipboard functions
#include "clipboard.h"

//Thread functions
#include "thread.h"

//Thread mutex functions
#include "mutex.h"

//A linear pool of resources
#include "resourcepool.h"

//A dynamic, recycling pool of resources
#include "recycledpool.h"

//Include various special pointers
#include "pointers.h"

//Random functionality (accessible via seashell::marsenne or seashell::rand)
#include "random.h"

//File enumeration functionality
#include "fileenumerator.h"

//Bitfield class - good for various compression algorithms and network apps
#include "bitfield.h"

//Bytefield class - good for file output, etc
#include "bytefield.h"

//Good string safe libraries
#ifdef _WINDOWS
#include <strsafe.h>
#else
#include "strsafe.h"
#endif //OS defines

#endif//SEASHELL_H_
