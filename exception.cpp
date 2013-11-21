#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "seashell.h"
#include "disablemmgrmacros.h"

Exception::Exception(const char* file, 
    suint line, 
    const char* func,
    const char* reason,
    ...)
{
    char* temp;
    makeVarargsString(temp, reason);
    init(file, line, func, "%s", temp);
    free(temp);
}



Exception::Exception(const Exception& e)
{
    //prevent a strdup
    Exception& other = const_cast<Exception&>(e);
    errorString = other.errorString;
    other.errorString = 0;
}



void Exception::init(const char* file, suint line, const char* func,
  const char* reason, ...)
{
#if PROFILE
    profilerFingerprint = profiler::getStackFingerprint();
#endif

    va_list args;

    char firstTry = 1;
    sint fileInfoSize = 0;
    sint strsize = 256;
    static const char* fileInfoString = "%s(%i)\n  %s\n\nException:\n    ";
    
    const sint tempSize = (sint)strlen(reason) + 
      (sint)strlen(fileInfoString) + 128;
    if (strsize < tempSize)
        strsize = tempSize;

    while (1) {
        errorString = (char*)malloc(strsize);
        if (firstTry) {
            firstTry = 0;
            StringCchPrintf(errorString, strsize, fileInfoString, file, 
              line, func);
            fileInfoSize = (sint)strlen(errorString);
        }

        const sint freeSpace = strsize - fileInfoSize;
        va_start(args, reason);
        sint result = vsnprintf(&errorString[fileInfoSize], freeSpace, 
          reason, args);
        if (result < 0 || result == freeSpace) { //chopped off
            strsize *= 2;
            free(errorString);
            continue;
        }
        break;
    }

    char replacedChar = errorString[fileInfoSize];
    StringCchPrintf(errorString, strsize, fileInfoString, file, line, func);
    errorString[fileInfoSize] = replacedChar;
}



Exception::~Exception()
{
    free(errorString);
}



void Exception::printStack(FILE* file) const
{
#if PROFILE
    profiler::printStackTrace(file, profilerFingerprint);
#endif
}



namespace exception
{

//We need to lock a mutex to prevent multiple concurrent write requests.
seashell::Mutex m;
    
    

#ifndef CONSOLE_OUTPUT
char logInit = 0;
//If there are no exceptions, present a blank log-file.
static class AutoInit
{
public:
    AutoInit() 
    {
        LockMutex(m);
        
        if (!logInit) {
            logInit = 1;
            FILE* f = fopen("Exceptions", "wb");
            fclose(f);
        }
    }
} initializer;
#endif



void elog_func(const char* file, sint line, const char* function,
  const Exception& exception)
{
    FILE* f;
    LockMutex(m);

#ifndef CONSOLE_OUTPUT
    if (!logInit) {
        logInit = 1;
        f = fopen("Exceptions", "wb");
    }
    else {
        f = fopen("Exceptions", "ab");
    }
#else
    f = stdout;
#endif

    if (f) {
        fprintf(f, "--------------------\n");
        fprintf(f, "%s(%i) : Exception caught and treated\n    %s\n", 
          file, line, 
          function);
        exception.printStack(f);
        fprintf(f, "%s\n", exception.what());
#ifndef CONSOLE_OUTPUT
        fclose(f);
#endif
    }
}

}



#if TESTING >= TESTLEVEL_CORE
TEST_BUDDY(exceptionThrowing)
{
    try {
        ethrow(Exception, "Testing exception %s: #%i!", "hello", 1);
    }
    catch (const Exception& e) {
        testAssert(0 != strstr(e.what(), "Testing exception hello: #1!"),
          "Exception message not as expected.  Test #1.");
    }

    try {
        ethrow(Exception, "Testing exception %c%i!", '#', 2);
    }
    catch (const Exception& e) {
        testAssert(0 != strstr(e.what(), "Testing exception #2!"),
          "Exception message not as expected.  Test #2.");
    }
}
END_TEST_BUDDY()
#endif //TESTING
