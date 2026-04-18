#ifndef WINCOMPAT_H
#define WINCOMPAT_H

/* this will act as a lightweight header-only compatibility layer
 * that provides some Windows-specific definitions and functions
 * to reduce the workload of porting in the future.
 */

#ifdef _WIN32

#include <Windows.h>
#include <tchar.h>

#else

#error Not implemented on current platform

// #include "compat/types.h"

#endif

#endif /* WINCOMPAT_H */
