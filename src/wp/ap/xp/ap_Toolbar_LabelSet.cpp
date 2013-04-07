/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <string.h>

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ev_Toolbar_Labels.h"
#include "xap_App.h"
#include "xap_Toolbar_ActionSet.h"
#include "ap_Toolbar_Id.h"
#include "ap_Strings.h"

EV_Toolbar_LabelSet * AP_CreateToolbarLabelSet(const char * szLanguage_)
{
	char buf[300];
	strcpy(buf,szLanguage_ ? szLanguage_ : "");
	char* szLanguage = buf;

	/* remove encoding part from locale name */
	char* dot = strrchr(szLanguage,'.');
	if (dot)
	{
		*dot = '\0'; 
	}

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	EV_Toolbar_LabelSet * pLabelSet = new EV_Toolbar_LabelSet(szLanguage,AP_TOOLBAR_ID__BOGUS1__,AP_TOOLBAR_ID__BOGUS2__);
	pLabelSet->setLabel(0 /*AP_TOOLBAR_ID_BOGUS1__*/, NULL, "NoIcon", NULL, NULL);

	UT_String iconname;
	
	#define toolbaritem(id) \
		iconname = #id; \
		iconname += "_"; \
		iconname += szLanguage; \
		pLabelSet->setLabel(	(AP_TOOLBAR_ID_##id), \
								pSS->getValue(AP_STRING_ID_TOOLBAR_LABEL_##id), \
								iconname.c_str(), \
								pSS->getValue(AP_STRING_ID_TOOLBAR_TOOLTIP_##id), \
								pSS->getValue(AP_STRING_ID_TOOLBAR_STATUSLINE_##id) );
		#include "ap_Toolbar_Id_List.h"
	#undef toolbaritem
	return pLabelSet;
}


