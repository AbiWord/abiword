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



#ifndef COLUMNBREAKER_H
#define COLUMNBREAKER_H

#include "ut_types.h"


class fp_Container;
class fl_DocSectionLayout;
class fp_Page;

class ABI_EXPORT fb_ColumnBreaker
{
public:
	fb_ColumnBreaker();
	virtual ~fb_ColumnBreaker(void) {}
	UT_sint32 breakSection(fl_DocSectionLayout * pSL);
	void   setStartPage(fp_Page * pPage) { m_pStartPage = pPage;} 
	fp_Page * getStartPage(void) { return m_pStartPage;}
private:
	bool _breakTable(fp_Container *& pOffendingContainer,
							fp_Container *& pLastContainerToKeep,
							int iMaxColHeight, int iWorkingColHeight,
							int iContainerMarginAfter);
	fp_Page * m_pStartPage;
};

#endif /* COLUMNBREAKER_H */
