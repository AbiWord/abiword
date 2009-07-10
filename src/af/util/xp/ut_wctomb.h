#ifndef UT_WCTOMB_H
#define UT_WCTOMB_H

#include <limits.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_iconv.h"

class ABI_EXPORT UT_Wctomb
{
public:

  void initialize();
  UT_Wctomb(const char* to_charset);
  ~UT_Wctomb();
  int wctomb(char * pC,int &length,UT_UCS4Char wc, int max_len = 100);
  void wctomb_or_fallback(char * pC,int &length,UT_UCS4Char wc, int max_len = 100);

  void setOutCharset(const char* charset);

  // TODO: make me private
  UT_Wctomb();

 private:
  // no implementations
  UT_Wctomb(const UT_Wctomb& v);
  UT_Wctomb& operator=(const UT_Wctomb &rhs);

  UT_iconv_t cd;
};

#endif // UT_WCTOMB_H
