
#ifndef UT_DEBUGMSG_H
#define UT_DEBUGMSG_H

void _UT_OutputMessage(char *s, ...);

#ifdef UT_DEBUG
#define UT_DEBUGMSG(M) _UT_OutputMessage M
#else
#define UT_DEBUGMSG(M)
#endif

#endif /* UT_DEBUGMSG_H */
