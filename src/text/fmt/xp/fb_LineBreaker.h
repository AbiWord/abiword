/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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



#ifndef FB_LINEBREAKER_H
#define FB_LINEBREAKER_H

#include "ut_types.h"

class fl_BlockLayout;
class fp_Line;
class fp_Run;
class fp_Page;
struct fp_RunSplitInfo;

/*
	fb_LineBreaker encapsulates knowledge of how to break runs across lines.  
	It also breaks them between words, to help with later justification. 
*/
class ABI_EXPORT fb_LineBreaker
{
public:
	fb_LineBreaker();
	virtual ~fb_LineBreaker(void);
	UT_sint32	breakParagraph(fl_BlockLayout*, fp_Line * pLineToStartAt, fp_Page * pPage);

protected:
	void		_breakTheLineAtLastRunToKeep(fp_Line *pLine, fl_BlockLayout *pBlock,fp_Page * pPage);
	UT_sint32	_moveBackToFirstNonBlankData(fp_Run *pCurrentRun, fp_Run **pOffendingRun);
	bool		_splitAtOrBeforeThisRun(fp_Run *pCurrentRun, UT_sint32 iTrailSpace);
	bool		_splitAtNextNonBlank(fp_Run *pCurrentRun);
	void		_splitRunAt(fp_Run *pCurrentRun, fp_RunSplitInfo &splitInfo);

private:
	fp_Run* 	m_pFirstRunToKeep;
	fp_Run* 	m_pLastRunToKeep;
	
	UT_sint32	m_iMaxLineWidth;
	UT_sint32	m_iWorkingLineWidth;
};

#endif /* FB_LINEBREAKER_H */
