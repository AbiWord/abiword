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

class ABI_EXPORT fp_AnnotationRun
    : public fp_HyperlinkRun
{
public:
    fp_AnnotationRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);
    virtual ~fp_AnnotationRun();
	virtual FP_HYPERLINK_TYPE getHyperlinkType(void) const override
        { return HYPERLINK_ANNOTATION; }
	UT_uint32 getPID(void) const
        { return m_iPID; }
	const char* getValue(void) const;
    void recalcValue(void);
	virtual bool canBreakAfter(void) const override;
	virtual bool canBreakBefore(void) const override;
	UT_sint32 getRealWidth(void) const
        { return m_iRealWidth; }
    void cleanDraw(dg_DrawArgs*);
	UT_sint32 calcWidth(void);

protected:
	virtual void _draw(dg_DrawArgs*) override;
	virtual void _clearScreen(bool bFullLineHeightRect) override;
	virtual bool _recalcWidth(void) override;
	bool _setValue(void);
	virtual void _setWidth(UT_sint32 iWidth) override;
	virtual bool _letPointPass(void) const override;
	virtual bool _canContainPoint(void) const override;
    virtual void _lookupProperties(const PP_AttrProp* pSpanAP,
                                   const PP_AttrProp* pBlockAP,
                                   const PP_AttrProp* pSectionAP,
								   GR_Graphics* pG) override;
 private:
	UT_uint32 m_iPID;
	UT_UTF8String m_sValue;
	UT_sint32 m_iRealWidth;
};
