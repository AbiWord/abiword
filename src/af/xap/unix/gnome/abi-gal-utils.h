/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * utils.h
 * Copyright 2000, 2001, Ximian, Inc.
 * Copyright 2003 Hubert Figuiere
 *
 * Maintainer:
 *   Hubert Figuiere <hfiguiere@teaser.fr>
 * Grafted for e-util.h and various other files in GAL and Gnumeric
 * for AbiWord.
 * Original Authors:
 *   Chris Lahey <clahey@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */


#ifndef __ABI_GAL_UTILS_H__
#define __ABI_GAL_UTILS_H__
#include <libintl.h>

#ifdef WITH_GNOME
#include <libgnome/gnome-i18n.h>
#else
#define _(String) gettext (String)
#define N_(String) (String)
#endif /* WITH_GNOME */

#define E_MAKE_TYPE(l,str,t,ci,i,parent) \
GType l##_get_type(void)\
{\
        static GType type = 0;                          \
        if (!type){                                     \
                static GTypeInfo const object_info = {  \
                        sizeof (t##Class),              \
                                                        \
                        (GBaseInitFunc) NULL,           \
                        (GBaseFinalizeFunc) NULL,       \
                                                        \
                        (GClassInitFunc) ci,            \
                        (GClassFinalizeFunc) NULL,      \
                        NULL,   /* class_data */        \
                                                        \
                        sizeof (t),                     \
                        0,      /* n_preallocs */       \
                        (GInstanceInitFunc) i,          \
                };                                      \
                type = g_type_register_static (parent, str, &object_info, 0);   \
        }                                               \
        return type;                                    \
}

#endif
