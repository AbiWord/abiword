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

#ifndef GR_WIN32IMAGE_H
#define GR_WIN32IMAGE_H

#include "gr_Image.h"

// fwd. decl.
typedef struct tagBITMAPINFO BITMAPINFO;


class GR_Win32Image : public GR_RasterImage
{
public:
	GR_Win32Image(const char* szName);
	~GR_Win32Image();

	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	
	void				setDIB(BITMAPINFO *pDIB) { m_pDIB = pDIB; }
	inline BITMAPINFO*	getDIB(void) const { return m_pDIB; }

protected:
	BITMAPINFO*		m_pDIB;
};

#endif /* GR_WIN32IMAGE_H */
