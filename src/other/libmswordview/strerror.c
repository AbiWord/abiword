
#include <stdio.h>

char *strerror(int errnum)
{
#ifdef BEOS
	// TODO replace this hack.
	// TODO i couldn't find the library with sys_errlist in it.
	static char buf[128];
	sprintf(buf,"System Error Number %d.",errnum);
	return buf;
#else
	extern char *sys_errlist[];
	return sys_errlist[errnum];
#endif
}
