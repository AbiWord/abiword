#ifndef GTYPE_DEF_H
#define GTYPE_DEF_H

#include <stdlib.h>

#define gchar char
#define gshort short
#define glong long
#define gint int
#define gboolean int

#define guchar unsigned char
#define gushort unsigned short
#define gulong unsigned long
#define guint unsigned int

#define gfloat float
#define gdouble double

#define gpointer void*
#define gconstpointer const void*

#define gint8 signed char
#define guint8 unsigned char
#define gint16 signed short
#define guint16 unsigned short
#define gint32 signed int
#define guint32 unsigned int

#ifdef FRIBIDI_INTERNAL_INCLUDE
#ifndef TRUE
#define TRUE 1
#define FRIBIDI_DEFINED_TRUE
#endif

#ifndef FALSE
#define FALSE 0
#define FRIBIDI_DEFINED_FALSE
#endif

#define USE_SIMPLE_MALLOC 1
#define g_malloc(n) malloc(n)
#define g_new(T, n) malloc(n*sizeof(T))
#define g_free(P) free(P)

#endif /* frbidi internal*/

#endif
