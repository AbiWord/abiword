extern char *sys_errlist[];

char *strerror(int errnum)
	{
	return sys_errlist[errnum];
	}
