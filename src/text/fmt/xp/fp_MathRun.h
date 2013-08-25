/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (c) 2004 Martin Sevior
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

#ifndef FP_MATHRUN_H
#define FP_MATHRUN_H

#include "fp_Run.h"

class fl_BlockLayout;
class GR_EmbedManager;
class FL_DocLayout;

class ABI_EXPORT fp_MathRun : public fp_Run
{
public:
	fp_MathRun(fl_BlockLayout* pBL, PT_BlockOffset iOffsetFirst,PT_AttrPropIndex indexAP,pf_Frag_Object* oh);
    virtual	~ fp_MathRun(void);

	virtual void			mapXYToPosition(UT_sint32 xPos, UT_sint32 yPos, PT_DocPosition& pos, bool& bBOL, bool& bEOL, bool &isTOC);
	virtual void 			findPointCoords(UT_uint32 iOffset, UT_sint32& x, UT_sint32& y, UT_sint32& x2, UT_sint32& y2, UT_sint32& height, bool& bDirection);
	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual bool			isSuperscript(void) const ;
	virtual bool			isSubscript(void)  const;
	virtual bool 			hasLayoutProperties(void) const;
	GR_EmbedManager *   getMathManager(void);

	virtual void            updateVerticalMetric();

	const char *            getDataID(void) const;
	UT_sint32               getUID(void) const
	{ return m_iMathUID;}
protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG = NULL);

	void                    _lookupLocalProperties();

	virtual void			_draw(dg_DrawArgs*);
	virtual void			_clearScreen(bool bFullLineHeightRect);
	virtual bool			_letPointPass(void) const;
	void                    _drawResizeBox(UT_Rect box);
	UT_sint32               _getLayoutPropFromObject(const char * szProp);
    bool                    _updatePropValuesIfNeeded(void);
	virtual	bool		    _recalcWidth(void);
	UT_sint32               m_iPointHeight;
	const PP_AttrProp *     m_pSpanAP;
	UT_uint32               m_iGraphicTick;
	const gchar *        m_pszDataID;
	UT_UTF8String           m_sMathML;
	GR_EmbedManager * 	    m_pMathManager;
	UT_sint32               m_iMathUID;
	PT_AttrPropIndex        m_iIndexAP;
	FL_DocLayout *          m_pDocLayout;
	bool                    m_bNeedsSnapshot;
	pf_Frag_Object*         m_OH;
};

#endif /* FP_MATHRUN_H */
