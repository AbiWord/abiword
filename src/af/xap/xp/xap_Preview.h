/* AbiSource Application Framework
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


#ifndef XAP_PREVIEW_H
#define XAP_PREVIEW_H

#include "ut_misc.h"
#include "ut_types.h"

#include "ev_EditBits.h"

#include "gr_Graphics.h"

class XAP_Preview
{
public:

	XAP_Preview(GR_Graphics * gc);
	virtual ~XAP_Preview(void);

	void					setWindowSize(UT_sint32, UT_sint32);
	inline UT_sint32		getWindowWidth(void) const { return m_iWindowWidth; };
	inline UT_sint32		getWindowHeight(void) const { return m_iWindowHeight; };

	// we probably don't need this one
/*
  inline GR_Graphics * 	getGraphicsContext(void) const { return m_graphics; };
*/

	// function triggered by platform events to handle any drawing
	virtual void			draw(void) = 0;
	// function to handle mouse down event.
	virtual void			onLeftButtonDown(UT_sint32 x, UT_sint32 y) {  };

	
protected:

	// TODO : 
	// later we might add some useful high-level macro-like drawing functions
	// for previews, like drawing page boundaries, etc.
	
	UT_sint32			m_iWindowHeight;
	UT_sint32			m_iWindowWidth;

	GR_Graphics *		m_gc;
	
};

#endif /* XAP_PREVIEW_H */
