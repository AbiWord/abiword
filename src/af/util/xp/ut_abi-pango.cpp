/* AbiWord
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ut_abi-pango.h"

/*!
  \param data: pointer to PangoGlyphString
  \param unused: not used, should be NULL
  Will free a single PagonGlyphString object; it is intended to be used
  in calls to g_list_foreach for freeing list of PangoGlyphString's
*/
void UT_free1PangoGlyphString(gpointer data, gpointer /*unused*/)
{
	pango_glyph_string_free(static_cast<PangoGlyphString*>(data));
}

/*!
  \param data: pointer to PangoItem
  \param unused: not used, should be NULL
  Will free a single PagonItem object; it is intended to be used
  in calls to g_list_foreach for freeing list of PangoItem's
*/
void UT_free1PangoItem(gpointer data, gpointer /*unused*/)
{
	pango_item_free(static_cast<PangoItem*>(data));
}


