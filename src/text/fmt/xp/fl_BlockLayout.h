 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#ifndef BLOCKLAYOUT_H
#define BLOCKLAYOUT_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "fl_Types.h"
#include "fl_Layout.h"

// number of DocPositions occupied by the block strux
#define fl_BLOCK_STRUX_OFFSET	1

class FL_DocLayout;
class fl_SectionLayout;
class fb_LineBreaker;
class fp_BlockSlice;
class fp_Line;
class fp_Run;
class DG_Graphics;
class PD_Document;
class PP_Property;
	
/*
	A BlockLayout is the object which is responsible for keeping track of 
	the layout information for a given block-level element, aka paragraph.
	This corresponds to CSS' notion of a "block box".  However, we don't
	call it a box, since it is not guaranteed, or even likely, to be
	rectangular.

	This object keeps track of all the CSS block properties which were 
	specified for this paragraph.

	Since a paragraph may be split across 2 or more pages, a fl_BlockLayout 
	may be split into slices.  Each slice is represented by an instance of 
	fp_BlockSlice, declared below.

	The fl_BlockLayout is responsible for managing margins, borders, padding, 
	background colors, first-line-indent, tabs, widow/orphan control and other 
	paragraph-level settings regarding the block element which is contained 
	within it.

	The fl_BlockLayout's primary function is managing the space required for 
	the layout of its paragraph.  It accomplishes this by acting as a proxy 
	for space requests from the fb_LineBreaker.  When the fb_LineBreaker asks 
	for space, using requestLineSpace, the fl_BlockLayout tries (if necessary) 
	to grow one of its BlockSlices enough to accomodate the line.  This 
	involves requesting room to grow from the fp_Column which contains the 
	slice.

	requestLineSpace is called by the fb_LineBreaker to ask the fl_BlockLayout 
	for space which will be filled with a fp_Line.  The iHeight argument 
	specifies how much height we need.  The return value is the maximum width 
	available for a line of the given height.  The underlying state is not 
	modified.  No space is actually reserved until the fp_Line is actually 
	added.

	Usually, the way to use this within a fb_LineBreaker is to guess the 
	height of the line by checking the height of the default font.  Then, ask 
	the fl_BlockLayout for the width of a line that high.  Go ahead and format 
	the line as if it were that wide.  After you're done, check to see if the 
	height of the resulting line is the same as (or smaller than, I suppose) 
	what you guessed.  If it's larger, you need to call requestLineSpace() to 
	make sure that the taller line is still okay at the width.  If not, you 
	have a problem.  If so, go ahead and add the line.  Don't add a line until 
	you know that it fits, or else addLine() will fail in a non-user-friendly 
	way.

	requestLineSpace works by finding the bottom of the last child fp_Line.
	Then, it tries to find space for a rectangle of the given height, 
	immediately below that last fp_Line.

	If the return value is 0, then there is no room for a line of that height.
*/
class fl_BlockLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	fl_BlockLayout(PL_StruxDocHandle sdh, fb_LineBreaker*, fl_BlockLayout*, fl_SectionLayout*);
	~fl_BlockLayout();

	void setNeedsReformat(UT_Bool);
	UT_Bool needsReformat();

	int format();
	int reformat();
	int	requestLineSpace(int iHeight);
	int	addLine(fp_Line*);

	const char*	getProperty(const XML_Char * pszName);
	void setAlignment(UT_uint32 iAlignCmd);
	UT_uint32 getAlignment();

	fl_BlockLayout* getNext() const;
	fl_BlockLayout* getPrev() const;
	fp_BlockSlice* getFirstSlice();
	fp_BlockSlice* getLastSlice();
	fp_Line* getFirstLine();
	fp_Line* getLastLine();
	fp_Line* findPrevLineInDocument(fp_Line*);
	fp_Line* findNextLineInDocument(fp_Line*);
	fp_Run* getFirstRun();

	FL_DocLayout * getLayout();
	UT_GrowBuf * getCharWidths(void);

	PT_DocPosition getPosition(UT_Bool bActualBlockPos=UT_FALSE) const;
	fp_Run* findPointCoords(PT_DocPosition position, UT_Bool bEOL, UT_uint32& x, UT_uint32& y, UT_uint32& height);

	UT_Bool getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	UT_Bool	getBlockBuf(UT_GrowBuf * pgb) const;

	UT_Bool truncateLayout(fp_Run* pTruncRun);

	void draw(DG_Graphics*);
	void clearScreen(DG_Graphics*);

	void dump();

	/*
		Blocks are stored in a linked list which contains all of the blocks in
		the normal flow, in order.  A fl_BlockLayout knows in which fp_Column each
		of its slices is, but it does not know the fp_Column-relative Y coordinate
		of any slice.  Rather, it simply knows which fl_BlockLayout is right before it
		in the normal flow.
	*/

protected:
	void _addSlice(fp_BlockSlice*);
	void _createNewSlice();
	void _createRuns();
	void _align();
	int						m_bNeedsReformat;
	void					_verifyCurrentSlice();
	UT_uint32				_getLastChar();
	void					_purgeLayout(UT_Bool bVisible);

	UT_GrowBuf				m_gbCharWidths;
	UT_Vector				m_vecSlices;

	FL_DocLayout*	       	m_pLayout;
	fb_LineBreaker*			m_pBreaker;

	fl_BlockLayout*			m_pPrev;
	fl_BlockLayout*			m_pNext;

	fp_BlockSlice*			m_pCurrentSlice;
	fp_Run*					m_pFirstRun;
	fl_SectionLayout*		m_pSectionLayout;

	fp_Line*				m_pFirstLine;
	fp_Line*				m_pLastLine;

	int						m_bFormatting;
};

#endif /* BLOCKLAYOUT_H */
