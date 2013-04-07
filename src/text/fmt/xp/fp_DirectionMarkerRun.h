/* AbiWord
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef FP_DIRMARKERRUN_H
#define FP_DIRMARKERRUN_H

#include "fp_Run.h"

#include "ut_types.h"
#include "ut_misc.h"

class ABI_EXPORT fp_DirectionMarkerRun : public fp_Run
{
public:
	fp_DirectionMarkerRun(fl_BlockLayout* pBL,
						  UT_uint32 iOffsetFirst,
						  UT_UCS4Char cMarker);

	virtual void			mapXYToPosition(UT_sint32 xPos,
											UT_sint32 yPos,
											PT_DocPosition& pos,
											bool& bBOL,
											bool& bEOL,
											bool & isTOC);

	virtual void 			findPointCoords(UT_uint32 iOffset,
											UT_sint32& x,
											UT_sint32& y,
											UT_sint32& x2,
											UT_sint32& y2,
											UT_sint32& height,
											bool& bDirection);

	virtual bool			canBreakAfter(void) const;
	virtual bool			canBreakBefore(void) const;
	virtual UT_sint32       getDrawingWidth() const { return static_cast<UT_sint32>(m_iDrawWidth);}

	// for the purposes of linebreaking, direction markers are just whitespace
	virtual bool		    doesContainNonBlankData(void) const { return false; }

protected:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG);

	virtual void			_draw(dg_DrawArgs*);
	virtual void       		_clearScreen(bool bFullLineHeightRect);
	virtual bool			_recalcWidth(void);
	virtual bool			_letPointPass(void) const;
	virtual bool            _deleteFollowingIfAtInsPoint() const;

private:
	UT_uint32				m_iXoffText;
	UT_uint32				m_iYoffText;
	UT_uint32				m_iDrawWidth;
	UT_UCS4Char             m_iMarker;
};

#endif
