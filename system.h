#ifndef CSMOD_SYSTEM_H
#define CSMOD_SYSTEM_H

#ifdef WIN32

#else

#   include <inttypes.h>
#   define DWORD int32_t
#   define UINT unsigned int

DWORD timeGetTime();

#endif

#endif // CSMOD_SYSTEM_H
