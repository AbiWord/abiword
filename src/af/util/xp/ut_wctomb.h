#ifndef UT_WCTOMB_H
#define UT_WCTOMB_H

#if defined(__FreeBSD__)
#include <cwchar.h>
#else
#include <wchar.h>
#endif
#include <string.h>
#include <limits.h>

#if defined(__BEOS__)
typedef int mbstate_t;
#endif

class UT_Wctomb
{
  mbstate_t m_state;
public:
  void initialize();
  UT_Wctomb();
  int wctomb(char * pC,int &length,wchar_t wc);
};

#endif // UT_WCTOMB_H
