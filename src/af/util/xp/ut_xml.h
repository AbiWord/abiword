/* Pseudoheader to include the right XML headers */

#ifdef HAVE_LIBXML2
#include <libxml/parser.h>
#else
#ifdef SHAREDLIB_EXPAT
#include "expat.h"
#else
#include "xmlparse.h"
#endif
#endif

