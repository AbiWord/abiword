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

#ifndef XAP_COCOAFONTMANAGER_H
#define XAP_COCOAFONTMANAGER_H

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"

#include "xap_CocoaFont.h"

/*****************************************************************/

class XAP_CocoaFontManager
{
public:
	XAP_CocoaFontManager(void);
	~XAP_CocoaFontManager(void);

	UT_Vector *			    getAllFonts(void);
	const XAP_CocoaFont *			getDefaultFont(void);
	const XAP_CocoaFont *			getFont(const char * fontname,
									XAP_CocoaFont::style s);
		
private:

	void					_addFont(const XAP_CocoaFont * font);
	void					_loadFontList ();

	UT_StringPtrMap 		m_fontHash;
	NSFontManager			*m_nsFontManager;
};

#endif /* XAP_COCOAFONTMANAGER_H */

