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

#include "gr_Win32Image.h"

#include "ut_bytebuf.h"

GR_Win32Image::GR_Win32Image(BITMAPINFO* pDIB, char* szName)
{
	m_pDIB = pDIB;

	if (szName)
	{
		strcpy(m_szName, szName);
	}
	else
	{
		strcpy(m_szName, "Win32Image");
	}
}

GR_Win32Image::~GR_Win32Image()
{
	delete m_pDIB;
}

UT_sint32	GR_Win32Image::getWidth(void) const
{
	return m_pDIB->bmiHeader.biWidth;
}

UT_sint32	GR_Win32Image::getHeight(void) const
{
	return m_pDIB->bmiHeader.biHeight;
}

void		GR_Win32Image::getByteBuf(UT_ByteBuf** ppBB) const
{
	UT_ByteBuf* pBB = new UT_ByteBuf();

#if 0
	// TODO convert to PNG and copy to the byte buf
#else
// ---- hack hack hack ----	
#define HACK_BYTES		"TODO convert this thing to PNG and store it here"
	
	pBB->ins(0, (const UT_Byte*) HACK_BYTES, sizeof(HACK_BYTES));
// ---- hack hack hack ----	
#endif
	
	*ppBB = pBB;
}

