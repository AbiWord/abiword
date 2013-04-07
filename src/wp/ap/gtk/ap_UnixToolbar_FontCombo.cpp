/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"

#include "ap_UnixToolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ev_UnixToolbar.h"
#include "ev_Toolbar.h"

#include "gr_CairoGraphics.h"

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
	UT_DEBUG_ONLY_ARG(id);
	UT_ASSERT(id == AP_TOOLBAR_ID_FMT_FONT);
	m_nPixels = 150;

	m_nLimit = GR_CairoGraphics::getAllFontCount();
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
	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	if(!pGF)
	{
		return false;
	}

	const std::vector<std::string>& names =
		GR_CairoGraphics::getAllFontNames();

	m_vecContents.clear();

	for (std::vector<std::string>::const_iterator i = names.begin(); 
		 i != names.end(); ++i)
	{
		const std::string & fName = *i;
		
		int foundAt = -1;

		for (UT_sint32 j = 0; j < m_vecContents.size(); j++)
		{
			// sort out dups
			const char * str = m_vecContents.getNthItem(j);
			if (str && (fName == str))
			{
				foundAt = j;
				break;
			}
		}

		if (foundAt == -1)
			m_vecContents.addItem(fName.c_str());
	}
	
	return true;
}
