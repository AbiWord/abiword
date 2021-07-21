/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
 * Copyright (C) 2021 Hubert Figuière
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

#pragma once

#include "fp_Run.h"

class ABI_EXPORT fp_FmtMarkRun
    : public fp_Run
{
public:
	fp_FmtMarkRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst);

	virtual void mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool & isTOC) override;
	virtual void findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection) override;
	virtual bool canBreakAfter(void) const override;
	virtual bool canBreakBefore(void) const override;
	virtual bool isSuperscript(void) const override;
	virtual bool isSubscript(void)  const override;
	virtual bool hasLayoutProperties(void) const override {return true;}

protected:
	virtual void _lookupProperties(const PP_AttrProp* pSpanAP,
                                   const PP_AttrProp* pBlockAP,
                                   const PP_AttrProp* pSectionAP,
                                   GR_Graphics* pG = nullptr) override;

	virtual void _draw(dg_DrawArgs*) override;
	virtual void _clearScreen(bool bFullLineHeightRect) override;
	virtual bool _letPointPass(void) const override;

private:
	enum
	{
		TEXT_POSITION_NORMAL,
		TEXT_POSITION_SUPERSCRIPT,
		TEXT_POSITION_SUBSCRIPT
	};
	UT_Byte m_fPosition;
};


