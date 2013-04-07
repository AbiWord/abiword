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

#ifndef GR_WIN32IMAGE_H
#define GR_WIN32IMAGE_H

#include "gr_Image.h"

// fwd. decl.
typedef struct tagBITMAPINFO BITMAPINFO;


class ABI_EXPORT GR_Win32Image : public GR_RasterImage
{
public:
	GR_Win32Image(const char* szName);
	virtual ~GR_Win32Image();

	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, const std::string& mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

	void				setDIB(BITMAPINFO *pDIB) { m_pDIB = pDIB; if (m_pDIB) setDisplaySize(m_pDIB->bmiHeader.biWidth,m_pDIB->bmiHeader.biHeight); }
	inline BITMAPINFO*	getDIB(void) const { return m_pDIB; }

	virtual GR_Image *  createImageSegment(GR_Graphics * pG, const UT_Rect & rec);
	virtual bool hasAlpha (void) const;
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y);

protected:
	BITMAPINFO*			m_pDIB;

private:
	bool				_convertFromPNG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	bool				_convertFromJPEG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
};

#endif /* GR_WIN32IMAGE_H */
