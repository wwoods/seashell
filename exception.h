//Walt Woods
//June 27th, 2007
//Standard exception class.

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <stdio.h>

/**Standard exception class designed for use with try {} catch {} blocks.
  *Typically, do not call this class' constructor manually.  Instead, use
  *ethrow(class, reason, etc) which automatically adds additional
  *information.
  */
class Exception
{
public:
    /**Copy constructor for passing through the stack.  De-const's the
      *parameter e, and transfers ownership of its error message.
      */
    Exception(const Exception& e);

    /**Blank constructor for derived classes; using init() before the derived
      *class' constructor finishes is mandatory.
      */
    Exception() {}

    /**Standard constructor.  Must be called by all derivatives of exceptions.
      * @param file File throwing exception.
      * @param line Line at which exception was thrown.
      * @param func Function throwing exception.
      * @param reason Text reason for exception.  Formatted akin to the 
      *printf() family of functions.  Additional arguments are expected to 
      *follow.
      * @param ... Additional arguments for the reason string.
      */
    Exception(const char* file,
        suint line,
        const char* func,
        const char* reason,
        ...);

    /**Free the reason string.
      */
    ~Exception();

    /** @return Returns the text reason for the exception.
      */
    const char* what() const { return errorString; }

    /**Prints the stack, as reported by the profiler, to the specified
      *file.  If PROFILE is disabled, then nothing will be printed.
      * @param file The stack will print into this file.
      */
    void printStack(FILE* file) const;

protected:    
    /**Exception's derived classes may use this function and the blank 
      *constructor to pass extended error information.
      */
    void init(const char* file,
        suint line,
        const char* func,
        const char* reason, ...);

private:
    /**Error string.
      */
    char* errorString;

#if PROFILE
    //Stack trace fingerprint obtained from profiler.
    void* profilerFingerprint;
#endif
};


/**Used by classes derived from Exception, takes a string parameter and the 
  *varargs portion of the function to allocate a string.
  * @param storage Name of char* variable to store the generated string in.
  * @param lastParam Name of last parameter preceeding the "...".  Assumed to be
  *the formatting string.
  */
#define makeVarargsString(storage, lastParam)\
    {\
        va_list args;\
        sint strsize = 32;\
        const sint tempSize = (sint)strlen(lastParam) + 32;\
        if (strsize < tempSize) \
            strsize = tempSize;\
        while (1) {\
            storage = (char*)malloc(strsize);\
\
            va_start(args, lastParam);\
            sint result = vsnprintf(storage, strsize, \
              lastParam, args);\
            if (result < 0 || result == strsize) { /*chopped off */ \
                strsize *= 2; \
                free(storage); \
                continue; \
            } \
            break; \
        }\
    }


namespace exception
{

/**Function used to log an exception when it has been successfully processed
  *and execution continues.
  */
void elog_func(const char* file, sint line, const char* function,
  const Exception& e);

} //exception



/**Macro used to throw exceptions.  Always compiled with code.
  */
#define ethrow(type, ...) { throw type(__FILE__, __LINE__, __FUNCTION__, \
  __VA_ARGS__); }

/**Macro used to call the logger.
  */
#define elog(e) exception::elog_func(__FILE__, __LINE__, __FUNCTION__, e)

/**Primarily for elog, makes an exception but does not throw it.
  */
#define makeException(...) Exception(__FILE__, __LINE__, __FUNCTION__, \
  __VA_ARGS__)

/**Macro used to throw an exception in debug mode only.
  */
#if ASSERTIONS
    #define eassert(cond, type, ...) { \
        if (!(cond)) ethrow(type, __VA_ARGS__); }
#else
    #define eassert(cond, type, ...) {}
#endif

//Debug macros that print short messages
#define QWE printf("%s:%i:%s\n", __FILE__, __LINE__, __FUNCTION__); fflush(stdout);
#define QWEO(t) QWE QWEO_(#t, t); fflush(stdout);
template<typename T>
void QWEO_(const char* name, const T& variable) {
    printf("%s {", name);
    for (sint i = 0; i < sizeof(T); i++) {
        if (i % 4 == 0)
            printf("\n  ");
        unsigned char value = *((unsigned char*)&variable + i);
        char value1 = (value >> 4);
        if (value1 < 10) value1 = '0' + value1;
        else value1 = 'a' + value1 - 10;
            
        char value2 = (value & 0x0f);
        if (value2 < 10) value2 = '0' + value2;
        else value2 = 'a' + value2 - 10;
            
        printf("%c%c ", value1, value2);
    }
    printf("}\n");
}

#endif//EXCEPTION_H_
