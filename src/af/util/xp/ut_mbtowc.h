#ifndef UT_MBTOWC_H
#define UT_MBTOWC_H

#include <limits.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_iconv.h"

class ABI_EXPORT UT_Mbtowc
{
public:
  void initialize();
  UT_Mbtowc();
  UT_Mbtowc(const char* from_charset);
  UT_Mbtowc(const UT_Mbtowc& v);
  ~UT_Mbtowc();  
  int mbtowc(UT_UCS4Char &wc,char mb);
  void setInCharset(const char* charset);
 private:
  char m_buf[MB_LEN_MAX];
  int m_bufLen;
  UT_iconv_t cd;
};

class ABI_EXPORT UT_UCS2_mbtowc
{
 private:
  class ABI_EXPORT Converter
    {
    public:
      Converter (const char * from_charset);

      ~Converter ();

      void initialize ();

      inline int   ref () { return ++m_count; }
      inline int unref () { return --m_count; }

      inline int count () const { return m_count; }

      inline const UT_iconv_t cd () const { return m_cd; }

    private:
      int m_count;

      UT_iconv_t m_cd;
    };
  Converter * m_converter;

 public:
  void initialize (bool clear = true);

  UT_UCS2_mbtowc ();
  UT_UCS2_mbtowc (const char * from_charset);
  UT_UCS2_mbtowc (const UT_UCS2_mbtowc & v);

  ~UT_UCS2_mbtowc();  

  void setInCharset (const char * from_charset);

  int mbtowc (UT_UCS2Char & wc, char mb);

 private:
  int m_bufLen;

  char m_buf[MB_LEN_MAX];
};

class ABI_EXPORT UT_UCS4_mbtowc
{
 private:
  class ABI_EXPORT Converter
    {
    public:
      Converter (const char * from_charset);

      ~Converter ();

      void initialize ();

      inline int   ref () { return ++m_count; }
      inline int unref () { return --m_count; }

      inline int count () const { return m_count; }

      inline const UT_iconv_t cd () const { return m_cd; }

    private:
      int m_count;

      UT_iconv_t m_cd;
    };
  Converter * m_converter;

 public:
  void initialize (bool clear = true);

  UT_UCS4_mbtowc ();
  UT_UCS4_mbtowc (const char * from_charset);
  UT_UCS4_mbtowc (const UT_UCS4_mbtowc & v);

  ~UT_UCS4_mbtowc();  

  void setInCharset (const char * from_charset);

  int mbtowc (UT_UCS4Char & wc, char mb);

 private:
  int m_bufLen;

  char m_buf[MB_LEN_MAX];
};

#endif // UT_MBTOWC_H
