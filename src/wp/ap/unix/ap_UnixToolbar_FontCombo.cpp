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
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"
#include "ap_UnixToolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"

#define MAX_FONTS 32767
#define PROPERTY_ARRAY_INCREMENT	16
#define XLFD_MAX_FIELD_LEN 64

/*****************************************************************/
EV_Toolbar_Control * AP_UnixToolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
														  AP_Toolbar_Id id)
{
	AP_UnixToolbar_FontCombo * p = new AP_UnixToolbar_FontCombo(pToolbar,id);
	return p;
}

AP_UnixToolbar_FontCombo::AP_UnixToolbar_FontCombo(EV_Toolbar * pToolbar,
													 AP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_FONT);

	m_nPixels = 175;		// TODO: do a better calculation
	m_nLimit = 32;			// TODO: honor this?  :)
}

AP_UnixToolbar_FontCombo::~AP_UnixToolbar_FontCombo(void)
{
	// nothing to purge.  contents are static strings
}

/*****************************************************************/

/* These are the array indices of the font properties used in several arrays,
   and should match the xlfd_index array below. */
typedef enum
{
  WEIGHT	= 0,
  SLANT		= 1,
  SET_WIDTH	= 2,
  SPACING	= 3,
  CHARSET	= 4,
  FOUNDRY	= 5
} PropertyIndexType;

/* These are the field numbers in the X Logical Font Description fontnames,
   e.g. -adobe-courier-bold-o-normal--25-180-100-100-m-150-iso8859-1 */
typedef enum
{
  XLFD_FOUNDRY		= 0,
  XLFD_FAMILY		= 1,
  XLFD_WEIGHT		= 2,
  XLFD_SLANT		= 3,
  XLFD_SET_WIDTH	= 4,
  XLFD_ADD_STYLE	= 5,
  XLFD_PIXELS		= 6,
  XLFD_POINTS		= 7,
  XLFD_RESOLUTION_X	= 8,
  XLFD_RESOLUTION_Y	= 9,
  XLFD_SPACING		= 10,
  XLFD_AVERAGE_WIDTH	= 11,
  XLFD_CHARSET		= 12
} FontField;

/* This is used to look up a field in a fontname given one of the above
   property indices. */
static const FontField xlfd_index[GTK_NUM_FONT_PROPERTIES] = {
  XLFD_WEIGHT,
  XLFD_SLANT,
  XLFD_SET_WIDTH,
  XLFD_SPACING,
  XLFD_CHARSET,
  XLFD_FOUNDRY
};

/*****************************************************************/

// These members are taken almost straight from the GTK font selector
// widget code.

gboolean AP_UnixToolbar_FontCombo::isXLFDFontName(const gchar *fontname)
{
	gint i = 0;
	gint field_len = 0;
  
	while (*fontname)
    {
		if (*fontname++ == '-')
        {
			if (field_len > XLFD_MAX_FIELD_LEN) return FALSE;
			field_len = 0;
			i++;
        }
		else
			field_len++;
    }
  
	return (i == 14) ? TRUE : FALSE;
}

gchar * AP_UnixToolbar_FontCombo::getFoundryFromXLFD(gchar * xlfd)
{
	gchar * chunk = new gchar[XLFD_MAX_FIELD_LEN];
	UT_ASSERT(chunk);

	*chunk = NULL;

	guint field_len = 0;
	guint i = 0;
	gchar * chunk_count = chunk;
	while (*xlfd && (i < 3))
	{
		if (*xlfd == '-')
		{
			field_len = 0;
			i++;
		}
		else
		{
			if ( i > 1 )
			{
				field_len++;
				*chunk_count = *xlfd;
				chunk_count++;
			}
		}
		xlfd++;
	}

	*chunk_count = NULL;
	return chunk;
}

UT_Bool AP_UnixToolbar_FontCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// is 100 a good guess when a user might have 2000 fonts?
	UT_HashTable stringTable(100);
	
	gchar **xfontnames;
	GSList **fontnames;
	gint num_fonts;
	gint i, prop;
  
	fontsel_info = g_new (GtkFontSelInfo, 1);
  
	/* Get a maximum of MAX_FONTS fontnames from the X server.
     Use "-*" as the pattern rather than "-*-*-*-*-*-*-*-*-*-*-*-*-*-*" since
     the latter may result in fonts being returned which don't actually exist.
     xlsfonts also uses "*" so I think it's OK. "-*" gets rid of aliases. */
	xfontnames = XListFonts (GDK_DISPLAY(), "-*", MAX_FONTS, &num_fonts);
	/* Output a warning if we actually get MAX_FONTS fonts. */
	if (num_fonts == MAX_FONTS)
		UT_DEBUGMSG(("MAX_FONTS exceeded. Some fonts may be missing."));
  
	fontsel_info->font_info = g_new (FontInfo, num_fonts);
	fontsel_info->pixel_sizes = g_new (guint16, num_fonts);
	fontsel_info->point_sizes = g_new (guint16, num_fonts);
  
	fontnames = g_new (GSList*, num_fonts);
  
	/* Create the initial arrays for the property value strings, though they
     may be realloc'ed later. Put the wildcard '*' in the first elements. */
	for (prop = 0; prop < GTK_NUM_FONT_PROPERTIES; prop++)
    {
		fontsel_info->properties[prop] = g_new(gchar*, PROPERTY_ARRAY_INCREMENT);
		fontsel_info->space_allocated[prop] = PROPERTY_ARRAY_INCREMENT;
		fontsel_info->nproperties[prop] = 1;
		fontsel_info->properties[prop][0] = "*";
    }
  
  
	fontsel_info->nfonts = 0;
	for (i = 0; i < num_fonts; i++)
    {
		if (isXLFDFontName(xfontnames[i]) != NULL)
		{
			gchar * fontName = getFoundryFromXLFD(xfontnames[i]);

			if (fontName)
			{
				// if it's not present
				if (!stringTable.findEntry(fontName))
					// add it
					stringTable.addEntry(fontName, fontName, fontName);
				else
					// wipe it from RAM
					delete [] fontName;
			}
		}
		else
			UT_DEBUGMSG(("Skipping invalid font: %s", xfontnames[i]));
    }
  

	// We can populate the family list with what's in our hash table
	int totalStringsInHash = stringTable.getEntryCount();
	for (int hashIndex = 0; hashIndex < totalStringsInHash; hashIndex++)
	{
		UT_HashTable::UT_HashEntry * item = stringTable.getNthEntry(hashIndex);

		if (item)
			if (item->pData)
				m_vecContents.addItem(item->pData);
			else
				UT_DEBUGMSG(("Hash item of index %d has NULL data", hashIndex));				
		else
			UT_DEBUGMSG(("Failed hash retrieval of index %d", hashIndex));
	}

	g_free(fontnames);
	XFreeFontNames(xfontnames);
	
	return UT_TRUE;
}
