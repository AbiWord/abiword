#include <wchar.h>
#include <string.h>
#include "ut_wctomb.h"

void UT_Wctomb::initialize()
{
  memset (&m_state, '\0', sizeof (m_state)); 
}

UT_Wctomb::UT_Wctomb()
{
  initialize();
}

#if defined(__QNXNTO__) || defined(__BEOS__)
#include <stdlib.h>

//We have to do this since wctomb clashes with the class name
int my_wctomb( char* s, wchar_t wchar, int *state ) {
	return wctomb(s, wchar);
}

#endif

int UT_Wctomb::wctomb(char * pC,int &length,wchar_t wc)
{
#if defined(__QNXNTO__) || defined(__BEOS__)
  size_t len=my_wctomb(pC,wc, &m_state);
#else
  size_t len=wcrtomb(pC,wc, &m_state);
#endif
  if(len==(size_t)-1)return 0;
  length=len;
  return 1;
}
