/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_debugmsg.h"

#include "ap_CocoaToolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_CocoaApp.h"
#include "xap_Frame.h"

#include "ev_CocoaToolbar.h"
#include "ev_Toolbar.h"


EV_Toolbar_Control * AP_CocoaToolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
								   XAP_Toolbar_Id tlbrid)
{
	AP_CocoaToolbar_FontCombo * p = new AP_CocoaToolbar_FontCombo(pToolbar,tlbrid);
	return p;
}

AP_CocoaToolbar_FontCombo::AP_CocoaToolbar_FontCombo(EV_Toolbar * pToolbar,
						     XAP_Toolbar_Id tlbrid)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_DEBUG_ONLY_ARG(tlbrid);

	UT_ASSERT(tlbrid == AP_TOOLBAR_ID_FMT_FONT);
	m_nPixels = 150;
	m_nLimit = 32;
}

AP_CocoaToolbar_FontCombo::~AP_CocoaToolbar_FontCombo(void)
{
	UT_VECTOR_FREEALL(char *, m_vecContents);
}

// this is the comparator to sort the combo box.
// TODO: move it else where in a place it is reusable by anybody.
static  int compareStrings(const void * ppS1, const void * ppS2)
{
	#warning TODO: move it else where in a place it is reusable by anybody.
	const char ** sz1 = (const char **) (ppS1);
	const char ** sz2 = (const char **) (ppS2);
	return strcmp(*sz1, *sz2);
}

bool AP_CocoaToolbar_FontCombo::populate(void)
{
    UT_ASSERT(m_pToolbar);

    // Things are relatively easy with the font manager.  Just
    // request all fonts and ask them their names.
    NSArray *list = [[NSFontManager sharedFontManager] availableFontFamilies];

    m_vecContents.clear();

    NSEnumerator* e = [list objectEnumerator];
    while(NSString *item = [e nextObject])
    {
        const char *fName = g_strdup([item UTF8String]);
        UT_ASSERT (fName);
        m_vecContents.addItem(fName);
    }
    m_vecContents.qsort(compareStrings);

    return true;
}
