/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_UNIXTOOLBAR_FONTCOMBO_H
#define AP_UNIXTOOLBAR_FONTCOMBO_H

#include <glib.h>
#include "xap_Types.h"
#include "ev_Toolbar_Control.h"
class EV_Toolbar;

/*****************************************************************/

typedef struct _FontInfo FontInfo;
struct _FontInfo
{
	gchar*	family;
	guint16	foundry;
	gint	style_index;
	guint16	nstyles;
};

typedef struct _FontStyle FontStyle;
struct _FontStyle
{
	guint16	properties[GTK_NUM_STYLE_PROPERTIES];
	gint	pixel_sizes_index;
	guint16	npixel_sizes;
	gint	point_sizes_index;
	guint16	npoint_sizes;
	guint8	flags;
};

typedef struct _GtkFontSelInfo GtkFontSelInfo;
struct _GtkFontSelInfo {
  
	/* This is a table with each FontInfo representing one font family+foundry */
	FontInfo *font_info;
	gint nfonts;
  
	/* This stores all the font sizes available for every style.
     Each style holds an index into these arrays. */
	guint16 *pixel_sizes;
	guint16 *point_sizes;
  
	/* These are the arrays of strings of all possible weights, slants, 
     set widths, spacings, charsets & foundries, and the amount of space
     allocated for each array. */

	gchar **properties[GTK_NUM_FONT_PROPERTIES];
	guint16 nproperties[GTK_NUM_FONT_PROPERTIES];
	guint16 space_allocated[GTK_NUM_FONT_PROPERTIES];
};

/*****************************************************************/

class AP_UnixToolbar_FontCombo : public EV_Toolbar_Control
{
public:
	AP_UnixToolbar_FontCombo(EV_Toolbar * pToolbar, AP_Toolbar_Id id);
	virtual ~AP_UnixToolbar_FontCombo(void);

	virtual UT_Bool		populate(void);

	static EV_Toolbar_Control * static_constructor(EV_Toolbar *, AP_Toolbar_Id id);

protected:
	gboolean isXLFDFontName(const gchar *fontname);
	gchar * getFoundryFromXLFD(gchar * xlfd);

	GtkFontSelInfo * fontsel_info;
		
};

#endif /* AP_UNIXTOOLBAR_FONTCOMBO_H */
