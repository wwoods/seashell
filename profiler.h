//Walt Woods
//June 27th, 2007
//Profiler class for all sorts of C++ applications.
//Designed for minimal perturbation of application runtime while still 
//possessing all necessary information.
//
//During execution, each thread has a separate profiler.  At thread/program
//termination, the profilers are merged into a primary application tree.
//
//
//Usage:
//Whatever function or code scope (Bracketed with {}) you want
//profiled, simply place the macro PROFILER("tag") at the top.
//For functions, using PROFILER(0) will simply output the 
//function name, which can be handy.
//
//
//Example:
//void myFunc()
//{
//  PROFILER(0); //Profiler entry "myFunc" created and timing
//  for (sint i = 0; i < 10000; i++) {
//    PROFILER("Inner Loop"); //Profiler entry "myFunc 'Inner Loop'"
//    ...
//  }
//}
//
//
//Output Format:
//The output is formatted in a fairly intuitive manner.  It is 
//important to note, however, that a ** prefix indicates an 
//extension of an entry's parent, and a *:: prefix indicates 
//that the namespaces and blasses are the same as its parent.
//
//
//Notes:
//To output information in 80-character width format to stdout
//(usually the console), define PROFILER_CONSOLE.
//
//Note that if this is compiled with MMGR, then each profiler
//section will have a memory usage tracker associated with it.

#ifndef PROFILER_H_
#define PROFILER_H_

//Define PROFILER_CONSOLE to output to console instead of file.
//#define PROFILER_CONSOLE

#if PROFILE
namespace profiler
{
//------------------------------------
//--    Needed Usage Information    --
//------------------------------------

#define PROFILER(t) static int profilerFingerprintMarker; profiler::Bomb \
  PROFBOMB192525((void*)&profilerFingerprintMarker, __FILE__, __LINE__, \
  __FUNCTION__, t);
#define PROFILER_RESET()
    
/** @return Returns a void* which may at any time after be passed to the 
  *function profiler::printStackTrace().  No memory is allocated; the return
  *value contains all necessary information.
  *
  *Note that the return value can be 0 if the profiler has been shut down.
  */
void* getStackFingerprint();

/**Prints the specified stack trace to the requested file.  Formatted as
  *follows:
  *
  *Application
  *File(Line)
  *  Function
  *File(Line)
  *  Function
  *etc..
  * @param file File to print stack trace to.  Presumed a logging file.
  * @param fingerprint Fingerprint of stack trace, as provided by a previous 
  *call to getStackTrace();
  */
void printStackTrace(FILE* file, void* fingerprint);

//--------------------------------
//--    Internal information    --
//--------------------------------

//Tells the profiler that the calling thread is about to be terminated.
void terminateThread();

struct TimingInfo;
class BombLocator;

/**Bomb is initialized with every PROFILER() tag, and destroyed on the 
  *function's exit.  Every time they are initialized, they inform their 
  *corresponding BombLocator that they need a new timing structure unique to 
  *that specific profile fingerprint (execution stack).
  */
class Bomb
{
    friend class ProfilerManager;

public: 
	/**Constructor acquires a timing module and initializes it.
      * @param fingerprint The key which corresponds to this Profiler bomb.
	  * @param file Name of source file.
	  * @param line Line in source file of ProfilerModule.
	  * @param function Function containing this ProfilerModule.
	  * @param name Name of this section of profiled code.
	  */
	Bomb(void* fingerprint,
      const char* file, suint line, const char* function, const char* name);

	/**Destructor stops the timing info.
	  */
	~Bomb();

private:
    //Current timing info for this bomb.
    TimingInfo* timingInfo_;
};



#define PROFILER_TERMINATE_THREAD() profiler::terminateThread();
} //Profiler

#else //No profiling
#define PROFILER(t)
#define PROFILER_RESET()
#define PROFILER_TERMINATE_THREAD()
#endif

#endif//PROFILER_H_
