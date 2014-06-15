#include "system.h"

#ifdef WIN32


#else

#include <time.h>

DWORD timeGetTime()
{
    timespec cls;
    clock_gettime(CLOCK_MONOTONIC, &cls);
    // second + nanoseconds
    return cls.tv_sec / 1000 + cls.tv_nsec * 10000000;
}


#endif
