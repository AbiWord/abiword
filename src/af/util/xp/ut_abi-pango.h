
#ifndef UT_ABI_PANGO_H
#define UT_ABI_PANGO_H

/*
   I have been having problems with including the pango/glib files; it seems that the layout created by
   ./configure, make, make install is different than the installed header files assume. This works for
   me, but something more protable will be needed later ...
*/

/*
   this is used internally by Pango and without it things do not work (I wish the documentation
   of Pango was bit better)
*/
#define PANGO_ENABLE_BACKEND
#include <glib-2.0/glib.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>

void UT_free1PangoGlyphString(gpointer data, gpointer /*unused*/);
void UT_free1PangoItem(gpointer data, gpointer /*unused*/);
#endif
