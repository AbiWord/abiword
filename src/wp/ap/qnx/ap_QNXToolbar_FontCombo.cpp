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

#include "ap_QNXToolbar_FontCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_QNXApp.h"
#include "xap_Frame.h"

#include "ev_QNXToolbar.h"
#include "ev_Toolbar.h"

#include <Pt.h>
#include <stdio.h>
#include <string.h>


EV_Toolbar_Control * AP_QNXToolbar_FontCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_QNXToolbar_FontCombo * p = new AP_QNXToolbar_FontCombo(pToolbar,id);
	return p;
}

AP_QNXToolbar_FontCombo::AP_QNXToolbar_FontCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/)
{
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_FONT);

	m_nPixels = 175;		// TODO: do a better calculation
	m_nLimit = 32;			// TODO: honor this?  :)
	m_vecContents.clear();
}

AP_QNXToolbar_FontCombo::~AP_QNXToolbar_FontCombo(void)
{
	printf("Purge the fonts! \n");
	// nothing to purge.  contents are static strings
}

#define FONT_QUERY_CHAR 'A'
#define FONT_TYPES (PHFONT_SCALABLE | PHFONT_BITMAP | PHFONT_PROP  /* | PHFONT_FIXED*/)
bool AP_QNXToolbar_FontCombo::populate(void)
{
	FontDetails *font_list;
	int			 index, alloc, count;

	UT_ASSERT(m_pToolbar);

	/* We should provide more than just the latin fonts */
	alloc = PfQueryFonts(FONT_QUERY_CHAR, FONT_TYPES, NULL, 0) + 10;

	if (!(font_list = (FontDetails *)malloc(alloc * sizeof(*font_list)))) {
		fprintf(stderr, "ERROR GETTING FONT LIST \n");
		return false;
	}
	memset(font_list, 0, alloc * sizeof(*font_list));

	if ((count = PfQueryFonts(FONT_QUERY_CHAR, FONT_TYPES, font_list, alloc)) < 0) {
		if (font_list) {
			free(font_list);
		}
		return false;
	}

	/* Now add the contents into the vector that we use as a reference */
	m_vecContents.clear();
	for (index = 0; index < count; index++) {
		if (*font_list[index].desc) {
			m_vecContents.addItem(strdup(font_list[index].desc));
		}
	}

	/* Is this a safe thing to do? */
	if (font_list) {
		free(font_list);
	}
	return true;
}
