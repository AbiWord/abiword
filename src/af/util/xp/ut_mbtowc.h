#ifndef UT_MBTOWC_H
#define UT_MBTOWC_H

#include <limits.h>

#if 0
/*
    old version using mbrtowc. Implementation used on *BSD systems is plain 
    wrong since it seems to always assume utf8 as mbs. It seems it won't work 
    for non-utf8 locales (e.g. CJK native encodings and non-latin1 singlebyte 
    encodings. - hvv@hippo.ru
*/

#if (! defined(__OpenBSD__)) && (! defined(__FreeBSD__))
#include <wchar.h>
#else
/* Note: wchar.h doesn't exist in Open/FreeBSD systems */
typedef int mbstate_t;
typedef unsigned long wchar_t;
size_t mbrtowc(wchar_t&,char*,int,mbstate_t);
#endif

#if defined(__BEOS__)
typedef int mbstate_t;
#endif

class UT_Mbtowc
{
 public:
  void initialize();
  UT_Mbtowc();
  int mbtowc(wchar_t &wc,char mb);
 private:
  char m_buf[MB_LEN_MAX];
  int m_bufLen;
  mbstate_t m_state;
};
#else

#include "ut_iconv.h"

class UT_Mbtowc
{
public:
  void initialize();
  UT_Mbtowc();
  UT_Mbtowc(const char* from_charset);
  UT_Mbtowc(const UT_Mbtowc& v);
  ~UT_Mbtowc();  
  int mbtowc(wchar_t &wc,char mb);
  void setInCharset(const char* charset);
 private:
  char m_buf[MB_LEN_MAX];
  int m_bufLen;
  UT_iconv_t cd;
};

#endif

#endif // UT_MBTOWC_H
