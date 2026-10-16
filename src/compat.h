/* macOS/POSIX compatibility for the original NetBSD Trek sources.
 * Local additions, 2026. Distributed under the BSD-3-Clause license.
 */
#ifndef TREK_COMPAT_H
#define TREK_COMPAT_H

#include <sys/cdefs.h>
#include <limits.h>
#include <float.h>
#ifndef __RCSID
#define __RCSID(s)
#endif
#ifndef __COPYRIGHT
#define __COPYRIGHT(s)
#endif
#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif
#ifndef __unused
#define __unused __attribute__((__unused__))
#endif
#ifndef __printflike
#define __printflike(f, a) __attribute__((__format__(__printf__, f, a)))
#endif
#ifndef RANDOM_MAX
#define RANDOM_MAX 2147483647L
#endif

#endif
