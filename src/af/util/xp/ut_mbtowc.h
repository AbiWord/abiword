#ifndef UT_MBTOWC_H
#define UT_MBTOWC_H

#include<wchar.h>
#include<limits.h>

class UT_Mbtowc
{
  char m_buf[MB_LEN_MAX];
  int m_bufLen;
  mbstate_t m_state;
public:
  void initialize();
  UT_Mbtowc();
  int mbtowc(wchar_t &wc,char mb);
};

#endif // UT_MBTOWC_H
