//Walt Woods
//August 5th, 2007
//Platform independent timing functions.

namespace timing
{

/**Function used to determine timings in milliseconds on various platforms.
  * @return Returns the system time, in milliseconds.  Has different meanings
  *on different operating systems.
  */
big_suint getSystemMs();

/**Retrieves the current thread's running cpu usage time, in milliseconds.  
  *Used for profiling purposes.
  * @return Returns the number of milliseconds that the current thread
  *has spent executing.
  */
big_suint getThreadExecutionMs();

/**DISABLED function - lack of portability in linux.
  *
  *Retrieves another thread's running cpu time, in milliseconds.
  * @param thread ThreadId of thread to query.
  */
//big_suint getOtherThreadExecutionMs(ThreadId thread);

/**Retrieves the current thread's running cpu usage time, in microseconds.  
  *Used for profiling purposes.
  * @return Returns the number of microseconds that the current thread
  *has spent executing.
  */
big_suint getThreadExecutionUs();

/**Sleeps a thread for the specified number of milliseconds.
  * @param ms Time, in milliseconds, to sleep the current thread for.
  */
void sleepThread(suint ms);

} //timing
