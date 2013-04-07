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



#ifndef COLUMNBREAKER_H
#define COLUMNBREAKER_H

#include "ut_types.h"


class fp_Container;
class fl_DocSectionLayout;
class fp_Page;
class fl_BlockLayout;

class ABI_EXPORT fb_ColumnBreaker
{
public:
	fb_ColumnBreaker(fl_DocSectionLayout * pDSL);
	virtual ~fb_ColumnBreaker(void) {}
	UT_sint32 breakSection(void);
	void   setStartPage(fp_Page * pPage);
	fp_Page * getStartPage(void) { return m_pStartPage;}
	fp_Page * needsRebreak(void);
private:
	fp_Page *               _getLastValidPage(void);
	UT_sint32               _breakSection(fp_Page * pStartPage);
	bool                    _checkVBreakableContainer(fp_Container * pContainer, UT_sint32 iAvail);

	fp_Container * _getNext(fp_Container * pCon);
	bool           _displayAnnotations(void);
	fp_Page *             m_pStartPage;
	bool                  m_bStartFromStart;
	bool                  m_bReBreak;
	fl_DocSectionLayout * m_pDocSec;
	fl_BlockLayout *      m_pCurrentBlock;
};

#endif /* COLUMNBREAKER_H */
