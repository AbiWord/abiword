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
#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"
#include "ap_BeOSToolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"

#include <Font.h>
#include <string.h>

#define MAX_FONTS 32767
#define PROPERTY_ARRAY_INCREMENT	16
#define XLFD_MAX_FIELD_LEN 64

/*****************************************************************/
EV_Toolbar_Control * AP_BeOSToolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_BeOSToolbar_FontCombo * p = new AP_BeOSToolbar_FontCombo(pToolbar,id);
	return p;
}

AP_BeOSToolbar_FontCombo::AP_BeOSToolbar_FontCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_FONT);

	m_nPixels = 100;		// TODO: do a better calculation
	m_nLimit = 32;			// TODO: honor this?  :)
}

AP_BeOSToolbar_FontCombo::~AP_BeOSToolbar_FontCombo(void)
{
	// nothing to purge.  contents are static strings
}

bool AP_BeOSToolbar_FontCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

  	int32 numFamilies = count_font_families(); 
   	for ( int32 i = 0; i < numFamilies; i++ ) { 
       font_family family;
       uint32 flags; 
       if (get_font_family(i, &family, &flags) == B_OK ) { 
           /*
           int32 numStyles = count_font_styles(family); 
           for ( int32 j = 0; j < numStyles; j++ ) { 
               font_style style; 
               if ( get_font_style(family, j, &style, &flags) 
                                                 == B_OK ) { 
                   . . . 
               } 
           }
           */ 
			char *new_family = new char[strlen(family) +1];
			strcpy(new_family, family);
			m_vecContents.addItem(new_family);
       } 
   	}

	return true;
}
