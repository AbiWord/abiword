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

class ABI_EXPORT UT_UCS2_mbtowc
{
 private:
  class ABI_EXPORT Converter
    {
    public:
      Converter (const char * from_charset);
      ~Converter ();

      void initialize ();

      inline const UT_iconv_t cd () const { return m_cd; }

    private:
      UT_iconv_t m_cd;
    };
  Converter * m_converter;

 public:
  void initialize (bool clear = true);

  UT_UCS2_mbtowc (const char * from_charset);
  ~UT_UCS2_mbtowc();  

  void setInCharset (const char * from_charset);

  int mbtowc (UT_UCS2Char & wc, char mb);

  // TODO: make me private
  UT_UCS2_mbtowc ();

 private:
  // no impls
  UT_UCS2_mbtowc (const UT_UCS2_mbtowc & v);
  UT_UCS2_mbtowc& operator=(const UT_UCS2_mbtowc &rhs);

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

      inline const UT_iconv_t cd () const { return m_cd; }

    private:
      UT_iconv_t m_cd;
    };
  Converter * m_converter;

 public:
  void initialize (bool clear = true);

  UT_UCS4_mbtowc (const char * from_charset);
  ~UT_UCS4_mbtowc();  

  void setInCharset (const char * from_charset);

  int mbtowc (UT_UCS4Char & wc, char mb);

  // TODO: make me private
  UT_UCS4_mbtowc ();

 private:
  // no impls
  UT_UCS4_mbtowc (const UT_UCS4_mbtowc & v);
  UT_UCS4_mbtowc& operator=(const UT_UCS4_mbtowc &rhs);

  int m_bufLen;
  char m_buf[MB_LEN_MAX];
};

#endif // UT_MBTOWC_H
