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
#include "ap_Frame.h"

static void    gtk_font_selection_insert_font        (GSList         *fontnames[],
						      gint           *ntable,
						      gchar          *fontname);
static gint    gtk_font_selection_insert_field       (gchar          *fontname,
						      gint            prop);

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

	m_nPixels = 150;		// TODO: do a better calculation
	m_nLimit = 32;
}

AP_UnixToolbar_FontCombo::~AP_UnixToolbar_FontCombo(void)
{
	// nothing to purge.  contents are static strings
}

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

static GtkFontSelInfo *fontsel_info = NULL;

#define MAX_FONTS 32767
#define PROPERTY_ARRAY_INCREMENT	16
#define XLFD_MAX_FIELD_LEN 64

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

#define GTK_NUM_FONT_PROPERTIES 6

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

// Taken almost straight from the GTK font selector dialog code
static gboolean gtk_font_selection_is_xlfd_font_name (const gchar *fontname)
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

gchar * getFoundryFromXLFD(gchar * xlfd)
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

static gchar*
gtk_font_selection_get_xlfd_field (const gchar *fontname,
				   FontField    field_num,
				   gchar       *buffer)
{
  const gchar *t1, *t2;
  gint countdown, len, num_dashes;
  
  if (!fontname)
    return NULL;
  
  /* we assume this is a valid fontname...that is, it has 14 fields */
  
  countdown = field_num;
  t1 = fontname;
  while (*t1 && (countdown >= 0))
    if (*t1++ == '-')
      countdown--;
  
  num_dashes = (field_num == XLFD_CHARSET) ? 2 : 1;
  for (t2 = t1; *t2; t2++)
    { 
      if (*t2 == '-' && --num_dashes == 0)
	break;
    }
  
  if (t1 != t2)
    {
      /* Check we don't overflow the buffer */
      len = (long) t2 - (long) t1;
      if (len > XLFD_MAX_FIELD_LEN - 1)
	return NULL;
      strncpy (buffer, t1, len);
      buffer[len] = 0;

      /* Convert to lower case. */
      g_strdown (buffer);
    }
  else
    strcpy(buffer, "(nil)");
  
  return buffer;
}


static void
gtk_font_selection_insert_font (GSList		      *fontnames[],
				gint		      *ntable,
				gchar		      *fontname)
{
  FontInfo *table;
  FontInfo temp_info;
  GSList *temp_fontname;
  gchar *family;
  gboolean family_exists = FALSE;
  gint foundry;
  gint lower, upper;
  gint middle, cmp;
  gchar family_buffer[XLFD_MAX_FIELD_LEN];
  
  table = fontsel_info->font_info;
  
  /* insert a fontname into a table */
  family = gtk_font_selection_get_xlfd_field (fontname, XLFD_FAMILY,
											  family_buffer);

  if (!family)
    return;
  
  foundry = gtk_font_selection_insert_field (fontname, FOUNDRY);
  
  lower = 0;
  if (*ntable > 0)
    {
      /* Do a binary search to determine if we have already encountered
       *  a font with this family & foundry. */
      upper = *ntable;
      while (lower < upper)
	{
	  middle = (lower + upper) >> 1;
	  
	  cmp = strcmp (family, table[middle].family);
	  /* If the family matches we sort by the foundry. */
	  if (cmp == 0)
	    {
	      family_exists = TRUE;
	      family = table[middle].family;
	      cmp = strcmp(fontsel_info->properties[FOUNDRY][foundry],
			   fontsel_info->properties[FOUNDRY][table[middle].foundry]);
	    }
	  
	  if (cmp == 0)
	    {
	      fontnames[middle] = g_slist_prepend (fontnames[middle],
						   fontname);
	      return;
	    }
	  else if (cmp < 0)
	    upper = middle;
	  else
	    lower = middle+1;
	}
    }
  
  /* Add another entry to the table for this new font family */
  temp_info.family = family_exists ? family : g_strdup(family);
  temp_info.foundry = foundry;
  temp_fontname = g_slist_prepend (NULL, fontname);
  
  (*ntable)++;
  
  /* Quickly insert the entry into the table in sorted order
   *  using a modification of insertion sort and the knowledge
   *  that the entries proper position in the table was determined
   *  above in the binary search and is contained in the "lower"
   *  variable. */
  if (*ntable > 1)
    {
      upper = *ntable - 1;
      while (lower != upper)
	{
	  table[upper] = table[upper-1];
	  fontnames[upper] = fontnames[upper-1];
	  upper--;
	}
    }
  table[lower] = temp_info;
  fontnames[lower] = temp_fontname;
}


/* This checks that the specified field of the given fontname is in the
   appropriate properties array. If not it is added. Thus eventually we get
   arrays of all possible weights/slants etc. It returns the array index. */
static gint
gtk_font_selection_insert_field (gchar		       *fontname,
				 gint			prop)
{
  gchar field_buffer[XLFD_MAX_FIELD_LEN];
  gchar *field;
  guint16 index2;
  
  field = gtk_font_selection_get_xlfd_field (fontname, xlfd_index[prop],
					     field_buffer);
  if (!field)
    return 0;
  
  /* If the field is already in the array just return its index2. */
  for (index2 = 0; index2 < fontsel_info->nproperties[prop]; index2++)
    if (!strcmp(field, fontsel_info->properties[prop][index2]))
      return index2;
  
  /* Make sure we have enough space to add the field. */
  if (fontsel_info->nproperties[prop] == fontsel_info->space_allocated[prop])
    {
      fontsel_info->space_allocated[prop] += PROPERTY_ARRAY_INCREMENT;
      fontsel_info->properties[prop] = g_realloc(fontsel_info->properties[prop],
						 sizeof(gchar*)
						 * fontsel_info->space_allocated[prop]);
    }
  
  /* Add the new field. */
  index2 = fontsel_info->nproperties[prop];
  fontsel_info->properties[prop][index2] = g_strdup(field);
  fontsel_info->nproperties[prop]++;
  return index2;
}

UT_Bool AP_UnixToolbar_FontCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// is 100 a good guess at the number of font families an
	// X user might have?
	UT_HashTable stringTable(100);
	
	gchar **xfontnames;
	GSList **fontnames;
	gint num_fonts;
	gint i, prop;
	gint npixel_sizes = 0, npoint_sizes = 0;
  
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
		if (gtk_font_selection_is_xlfd_font_name (xfontnames[i]) != NULL)
		{
//			gtk_font_selection_insert_font(fontnames, &fontsel_info->nfonts, xfontnames[i]);
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
			
/*
			if (fontName)
				m_vecContents.addItem(fontName);
			else
				UT_DEBUGMSG(("No family name could be found in font %s", xfontnames[i]));
*/
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
	
	
	/* Since many font names will be in the same FontInfo not all of the
     allocated FontInfo table will be used, so we will now reallocate it
     with the real size. */
	fontsel_info->font_info = (FontInfo *) g_realloc(fontsel_info->font_info,
													 sizeof(FontInfo) * fontsel_info->nfonts);
  
	fontsel_info->pixel_sizes = (guint16 *) g_realloc(fontsel_info->pixel_sizes,
													  sizeof(guint16) * npixel_sizes);
	fontsel_info->point_sizes = (guint16 *) g_realloc(fontsel_info->point_sizes,
													  sizeof(guint16) * npoint_sizes);
	g_free(fontnames);
	XFreeFontNames (xfontnames);
	
	return UT_TRUE;
}
