/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the wait3 system call.  */
#define HAVE_WAIT3 1

/* Define if you have the waitpid system call.  */
/* #undef HAVE_WAITPID */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the <errno.h> header file.  */
#define HAVE_ERRNO_H 1

#define HAVE_POSIX_SIGNALS 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H 

/* Define if you want zlib to uncompress wmf files */
#undef SYSTEM_ZLIB

/*signal things*/
#include <signal.h>

#if !defined (HAVE_POSIX_SIGNALS)
#  define signal_handle(sig, handler) signal (sig, handler)
#else
extern void signal_handle();   /* in c code*/
#if !defined (SA_RESTART)
#  define SA_RESTART 0
#endif
#endif /* _POSIX_VERSION */

typedef RETSIGTYPE SigHandler ();


#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdarg.h>

#include "getopt.h"

