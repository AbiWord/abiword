#ifndef UT_MBTOWC_H
#define UT_MBTOWC_H


#if (! defined(__OpenBSD__)) && (! defined(__FreeBSD__))
#include <wchar.h>
#else
/* Note: wchar.h doesn't exist in Open/FreeBSD systems */
typedef int mbstate_t;
typedef unsigned long wchar_t;
size_t mbrtowc(wchar_t&,char*,int,mbstate_t);
#endif

#include <limits.h>

#if defined(__BEOS__)
typedef int mbstate_t;
#endif

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
