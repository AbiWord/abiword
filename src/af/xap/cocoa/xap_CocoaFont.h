/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_COCOAFONT_H
#define XAP_COCOAFONT_H

#import <Cocoa/Cocoa.h>

#include "gr_Graphics.h"

/*
  We derive our handle from GR_Font so we can be passed around the GR
  contexts as a native Cocoa font.
  GR_Font is a completely virtual class with no data or accessors of
  its own.
*/

class XAP_CocoaFont : public GR_Font
{
 public:
	XAP_CocoaFont();
	XAP_CocoaFont(NSFont *font);
	XAP_CocoaFont(const XAP_CocoaFont & copy);
	~XAP_CocoaFont();

	NSFont * 		getNSFont(void) const
							{ return m_font; } ;
	UT_uint32		getSize(void);
	const char * 			getName(void);
	
	float						getAscent();
	float						getDescent(); /* returns -descent because it is <0 on CG */
	float						getHeight();
private:
	NSFont*						m_font;
	void						_resetMetricsCache();
	/* metrics cache */
	UT_uint32					_m_size;
	float						_m_ascent;
	float						_m_descent;
	float						_m_height;
};

#endif /* XAP_COCOAFONT_H */
