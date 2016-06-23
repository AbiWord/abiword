

#ifndef __XAP_GTK_STYLE_H
#define __XAP_GTK_STYLE_H

#include <gtk/gtk.h>

GtkStyleContext *
XAP_GtkStyle_get_style (GtkStyleContext *parent,
                        const char      *selector);


#endif
