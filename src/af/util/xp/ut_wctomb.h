#ifndef UT_WCTOMB_H
#define UT_WCTOMB_H

#include<wchar.h>
#include<string.h>
#include<limits.h>

class UT_Wctomb
{
  mbstate_t m_state;
public:
  void initialize();
  UT_Wctomb();
  int wctomb(char * pC,int &length,wchar_t wc);
};

#endif // UT_WCTOMB_H
