#include<wchar.h>
#include<string.h>
#include"ut_wctomb.h"

void UT_Wctomb::initialize()
{
  memset (&m_state, '\0', sizeof (m_state)); 
}

UT_Wctomb::UT_Wctomb()
{
  initialize();
}

int UT_Wctomb::wctomb(char * pC,int &length,wchar_t wc)
{
  size_t len=wcrtomb(pC,wc, &m_state);
  if(len==(size_t)-1)return 0;
  length=len;
  return 1;
}
