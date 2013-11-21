
#ifdef _WINDOWS

#include <windows.h>
#pragma comment(lib, "winmm.lib")

#elif defined(_LINUX)

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#endif //Operating system types

#include "seashell.h"

namespace timing
{

big_suint getSystemMs()
{
#ifdef _WINDOWS
	return (big_suint)timeGetTime();
#elif defined(_LINUX)
	timeval ts; gettimeofday(&ts, 0);
	return (big_suint)(ts.tv_sec * 1000 + ts.tv_usec / 1000);
#endif //Operating Systems
}

big_suint getThreadExecutionMs()
{
#ifdef _WINDOWS
    eassert(sizeof(FILETIME) == sizeof(big_suint), Exception,
      "Assumption about size of FILETIME has failed.  Sizeof(FILETIME) = "
      "%i, sizeof(big_suint) = %i", sizeof(FILETIME), sizeof(big_suint));

	FILETIME kernel;
	FILETIME user;
	FILETIME u; //useless
	GetThreadTimes(GetCurrentThread(), &u, &u, &kernel, &user);

	//values in 100-nanosecond intervals
	big_suint a = 0;
	//memcpy(&a, &kernel, sizeof(big_suint));
	big_suint b;
	memcpy(&b, &user, sizeof(big_suint));
	return (a + b) / 10000;
#elif defined(_LINUX)
	rusage r;
	getrusage(RUSAGE_SELF, &r);
	return (big_suint)(r.ru_utime.tv_sec * 1000 + r.ru_utime.tv_usec / 1000);
#endif //Operating Systems
}

big_suint getThreadExecutionUs()
{
#ifdef _WINDOWS
    eassert(sizeof(FILETIME) == sizeof(big_suint), Exception,
      "Assumption about size of FILETIME has failed.  Sizeof(FILETIME) = "
      "%i, sizeof(big_suint) = %i", sizeof(FILETIME), sizeof(big_suint));

	FILETIME kernel;
	FILETIME user;
	FILETIME u; //useless
	GetThreadTimes(GetCurrentThread(), &u, &u, &kernel, &user);

	//values in 100-nanosecond intervals
	big_suint a = 0;
	//memcpy(&a, &kernel, sizeof(big_suint));
	big_suint b = 0;
	memcpy(&b, &user, sizeof(big_suint));
	return (a + b) / 10;
#elif defined(_LINUX)
	rusage r;
	getrusage(RUSAGE_SELF, &r);
	return (big_suint)(r.ru_utime.tv_sec * 1000000 + r.ru_utime.tv_usec);
#endif //Operating Systems
}

#ifdef _WINDOWS
void sleepThread(suint ms) { Sleep((DWORD)ms); }
#elif defined(_LINUX)
void sleepThread(suint ms) { usleep(ms * 1000); }
#endif //Operating Systems

} //timing
