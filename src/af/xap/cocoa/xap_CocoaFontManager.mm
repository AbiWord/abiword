/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#import <AppKit/AppKit.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaFontManager.h"
#include "xap_EncodingManager.h"
#include "ut_string_class.h"
#include <sys/stat.h>
#include "xap_Strings.h"
#include "xap_Prefs.h"


XAP_CocoaFontManager::XAP_CocoaFontManager(void) : m_fontHash(256)
{
}

XAP_CocoaFontManager::~XAP_CocoaFontManager(void)
{
	UT_HASH_PURGEDATA(XAP_CocoaFont *, &m_fontHash, delete);
}


/*!
 * compareFontNames this function is used to compare the char * strings names
 * of the Fonts with the qsort method on UT_Vector.
\params const void * vF1  - Pointer to XAP_CocoaFont pointer
\params const void * vF2  - Pointer to a XAP_CocoaFont pointer
\returns -1 if sz1 < sz2, 0 if sz1 == sz2, +1 if sz1 > sz2
*/
static UT_sint32 compareFontNames(const void * vF1, const void * vF2)
{
	XAP_CocoaFont ** pF1 = (XAP_CocoaFont **) const_cast<void *>(vF1);
	XAP_CocoaFont ** pF2 = (XAP_CocoaFont **) const_cast<void *>(vF2);
	return UT_strcmp((*pF1)->getName(), (*pF2)->getName());
}

UT_Vector * XAP_CocoaFontManager::getAllFonts(void)
{
	UT_Vector * pVec = m_fontHash.enumerate();
	UT_ASSERT(pVec);
	if(pVec->getItemCount()> 1)
		pVec->qsort(compareFontNames);

	return pVec;
}

const XAP_CocoaFont * XAP_CocoaFontManager::getDefaultFont(void)
{
	// this function (perhaps incorrectly) always assumes
	// it will be able to find this font on the display.
	// this is probably not such a bad assumption, since
	// gtk itself uses it all over (and it ships with every
	// X11R6 I can think of)
	UT_DEBUGMSG(("XAP_CocoaFontManager::getDefaultFont\n"));

	XAP_CocoaFont * f = new XAP_CocoaFont();

	// do some manual behind-the-back construction
	f->setName("Default");
	f->setStyle(XAP_CocoaFont::STYLE_NORMAL);

	return f;
}


const XAP_CocoaFont * XAP_CocoaFontManager::getFont(const char * fontname,
											XAP_CocoaFont::style s)
{
	// We use a static buffer because this function gets called a lot.
	// Doing an allocation is really slow on many machines.
	static char keyBuffer[512];

	char * copy;
	UT_cloneString(copy, fontname);
	UT_upperString(copy);
	
	snprintf(keyBuffer, 512, "%s@%d", copy, s);

	FREEP(copy);
	
	const XAP_CocoaFont * entry = static_cast<const XAP_CocoaFont *>(m_fontHash.pick(keyBuffer));

	//UT_DEBUGMSG(("Found font [%p] in table.\n", entry));
	
	return const_cast<XAP_CocoaFont *>(entry);
}

	  
void XAP_CocoaFontManager::_addFont(const XAP_CocoaFont * newfont)
{
	// we index fonts by a combined "name" and "style"
	const char* fontkey = newfont->getFontKey();
	const void * curfont_entry = m_fontHash.pick(fontkey);
	if (curfont_entry)
	{
		const XAP_CocoaFont* curfont = static_cast<const XAP_CocoaFont*>(curfont_entry);
		delete curfont;     
		m_fontHash.remove (fontkey, NULL);
	} 

	/* 
	   since "standard fonts" folder is read first and then
	   current charset-specific subdirectory, it's obvious that
	   the recent version is current charset-specific font, 
	   so we replace original font (that is standard) 
	   unconditionally.
	*/

	m_fontHash.insert(fontkey,
			  (void *) newfont);		
}



