
#if defined(_WINDOWS)
#include <windows.h>
#elif defined(_LINUX)
#include <unistd.h>
#endif

#include "seashell.h"

namespace seashell
{

namespace systeminfo
{

sint getActiveProcessors()
{
    sint active = 0;
#if defined(_WINDOWS)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    active = (sint)info.dwNumberOfProcessors;
#elif defined(_LINUX)
    #ifdef _SC_NPROCESSORS_ONLN
        active = sysconf(_SC_NPROCESSORS_ONLN);
    #else
        active = 0;
    #endif
#endif

    return active;
}

} //systeminfo

} //seashell
