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

#include "xap_UnixFont.h"
#include "xap_UnixFontManager.h"

#include "ap_UnixToolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ev_UnixToolbar.h"
#include "ev_Toolbar.h"


EV_Toolbar_Control * AP_UnixToolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_UnixToolbar_FontCombo * p = new AP_UnixToolbar_FontCombo(pToolbar,id);
	return p;
}

AP_UnixToolbar_FontCombo::AP_UnixToolbar_FontCombo(EV_Toolbar * pToolbar,
												   XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id == AP_TOOLBAR_ID_FMT_FONT);
	m_nPixels = 150;
	m_nLimit = 32;
}

AP_UnixToolbar_FontCombo::~AP_UnixToolbar_FontCombo(void)
{
	// nothing to purge.  contents are static strings
}

bool AP_UnixToolbar_FontCombo::populate(void)
{
	UT_ASSERT(m_pToolbar);
	
	// Things are relatively easy with the font manager.  Just
	// request all fonts and ask them their names.
	EV_UnixToolbar * toolbar = static_cast<EV_UnixToolbar *>(m_pToolbar);
	
	UT_Vector * list = toolbar->getApp()->getFontManager()->getAllFonts();
	UT_ASSERT(list);

	UT_uint32 count = list->size();

	m_vecContents.clear();

	for (UT_uint32 i = 0; i < count; i++)
	{
		// sort-out duplicates
		XAP_UnixFont * pFont = (XAP_UnixFont *)list->getNthItem(i);
		const char * fName = pFont->getName();

		int foundAt = -1;

		for (int j = 0; j < m_vecContents.size(); j++)
		{
			// sort out dups
			char * str = (char *)m_vecContents.getNthItem(j);
			if (str && !UT_strcmp (str, fName))
			{
				foundAt = j;
				break;
			}
		}

		if (foundAt == -1)
			m_vecContents.addItem((void *)(fName));
	}
	DELETEP(list);

	return true;
}
