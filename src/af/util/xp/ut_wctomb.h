#ifndef UT_WCTOMB_H
#define UT_WCTOMB_H

#include <limits.h>
#include "ut_types.h"

#if 0
/*
    old version using wcrtomb. Implementation used on *BSD systems is plain
    wrong since it seems to always assume utf8 as mbs. It seems it won't work
    for non-utf8 locales (e.g. CJK native encodings and non-latin1 singlebyte
    encodings. - hvv@hippo.ru
*/


#if (! defined(__OpenBSD__)) && (! defined(__FreeBSD__))
#include <wchar.h>
#endif
#include <string.h>

#if defined(__BEOS__) || defined(__OpenBSD__) || defined(__FreeBSD__)
typedef int mbstate_t;
#endif

#if  defined(__OpenBSD__) || defined (__FreeBSD__)
typedef unsigned long UT_UCS4Char;
size_t wcrtomb(char *,UT_UCS4Char,mbstate_t *);
#endif

class ABI_EXPORT UT_Wctomb
{
public:
  void initialize();
  UT_Wctomb();
  int wctomb(char * pC,int &length,UT_UCS4Char wc);
 private:
  mbstate_t m_state;
};
#else /*portable version using iconv*/

#include "ut_iconv.h"

class ABI_EXPORT UT_Wctomb
{
public:
  void initialize();
  UT_Wctomb();
  UT_Wctomb(const char* to_charset);
  UT_Wctomb(const UT_Wctomb& v);
  ~UT_Wctomb();
  int wctomb(char * pC,int &length,UT_UCS4Char wc);
  void wctomb_or_fallback(char * pC,int &length,UT_UCS4Char wc);

  void setOutCharset(const char* charset);
 private:
  UT_iconv_t cd;
};
#endif
#endif // UT_WCTOMB_H
