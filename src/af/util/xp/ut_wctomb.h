#ifndef UT_WCTOMB_H
#define UT_WCTOMB_H

#include <limits.h>
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
typedef unsigned long wchar_t;
size_t wcrtomb(char *,wchar_t,mbstate_t *);
#endif

class UT_Wctomb
{
  mbstate_t m_state;
public:
  void initialize();
  UT_Wctomb();
  int wctomb(char * pC,int &length,wchar_t wc);
};
#else /*portable version using iconv*/

#include <iconv.h>

class UT_Wctomb
{
  iconv_t cd;
public:
  void initialize();
  UT_Wctomb();
  UT_Wctomb(const char* to_charset);
  UT_Wctomb(const UT_Wctomb& v);  
  ~UT_Wctomb();  
  int wctomb(char * pC,int &length,wchar_t wc);
  void wctomb_or_fallback(char * pC,int &length,wchar_t wc);  
  
  void setOutCharset(const char* charset);
};
#endif
#endif // UT_WCTOMB_H
