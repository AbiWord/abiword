 
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

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_DocLayout;
class FL_SectionLayout;
class FB_LineBreaker;
class FP_BlockSlice;
class FP_Line;
class FP_Run;
class DG_Graphics;
class PD_Document;
class PP_Property;

// TODO the following should be an enum
#define DG_ALIGN_BLOCK_LEFT		1
#define DG_ALIGN_BLOCK_RIGHT    2
#define DG_ALIGN_BLOCK_CENTER   3
#define DG_ALIGN_BLOCK_JUSTIFY  4
	
/*
	A BlockLayout is the object which is responsible for keeping track of 
	the layout information for a given block-level element, aka paragraph.
	This corresponds to CSS' notion of a "block box".  However, we don't
	call it a box, since it is not guaranteed, or even likely, to be
	rectangular.

	This object keeps track of all the CSS block properties which were 
	specified for this paragraph.

	Since a paragraph may be split across 2 or more pages, a FL_BlockLayout 
	may be split into slices.  Each slice is represented by an instance of 
	FP_BlockSlice, declared below.

	The FL_BlockLayout is responsible for managing margins, borders, padding, 
	background colors, first-line-indent, tabs, widow/orphan control and other 
	paragraph-level settings regarding the block element which is contained 
	within it.

	The FL_BlockLayout's primary function is managing the space required for 
	the layout of its paragraph.  It accomplishes this by acting as a proxy 
	for space requests from the FB_LineBreaker.  When the FB_LineBreaker asks 
	for space, using requestLineSpace, the FL_BlockLayout tries (if necessary) 
	to grow one of its BlockSlices enough to accomodate the line.  This 
	involves requesting room to grow from the FP_Column which contains the 
	slice.

	requestLineSpace is called by the FB_LineBreaker to ask the FL_BlockLayout 
	for space which will be filled with a FP_Line.  The iHeight argument 
	specifies how much height we need.  The return value is the maximum width 
	available for a line of the given height.  The underlying state is not 
	modified.  No space is actually reserved until the FP_Line is actually 
	added.

	Usually, the way to use this within a FB_LineBreaker is to guess the 
	height of the line by checking the height of the default font.  Then, ask 
	the FL_BlockLayout for the width of a line that high.  Go ahead and format 
	the line as if it were that wide.  After you're done, check to see if the 
	height of the resulting line is the same as (or smaller than, I suppose) 
	what you guessed.  If it's larger, you need to call requestLineSpace() to 
	make sure that the taller line is still okay at the width.  If not, you 
	have a problem.  If so, go ahead and add the line.  Don't add a line until 
	you know that it fits, or else addLine() will fail in a non-user-friendly 
	way.

	requestLineSpace works by finding the bottom of the last child FP_Line.
	Then, it tries to find space for a rectangle of the given height, 
	immediately below that last FP_Line.

	If the return value is 0, then there is no room for a line of that height.
*/
class FL_BlockLayout : public fl_Layout
{
	friend class fl_DocListener;

public:
	FL_BlockLayout(PL_StruxDocHandle sdh, FB_LineBreaker*, FL_BlockLayout*, FL_SectionLayout*);
	~FL_BlockLayout();

	void setNeedsReformat(UT_Bool);
	UT_Bool needsReformat();

	int format();
	int reformat();
	int	requestLineSpace(int iHeight);
	int	addLine(FP_Line*);

	const char*	getProperty(const PP_Property* pProp);
	void setAlignment(UT_uint32 iAlignCmd);
	UT_uint32 getAlignment();

	FL_BlockLayout* getNext() const;
	FL_BlockLayout* getPrev() const;
	FP_BlockSlice* getFirstSlice();
	FP_BlockSlice* getLastSlice();
	FP_Line* getFirstLine();
	FP_Line* getLastLine();
	FP_Line* findPrevLineInDocument(FP_Line*);
	FP_Line* findNextLineInDocument(FP_Line*);
	FP_Run* getFirstRun();

	UT_GrowBuf * getCharWidths(void);

	PT_DocPosition getPosition() const;
	FP_Run* findPointCoords(PT_DocPosition position, UT_Bool bRight, UT_uint32& x, UT_uint32& y, UT_uint32& height);

	UT_Bool getSpanPtr(UT_uint32 offset, const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;

#ifdef BUFFER	// top-down edit operations -- obsolete?
	UT_Bool		insertData(UT_UCSChar * text, UT_uint32 count);
	UT_Bool		cmdCharDelete(UT_Bool bForward, UT_uint32 iCount);

	void		mergeWithNextBlock();
	void		insertParagraphBreak();
	UT_Bool		insertInlineMarker(const XML_Char * szName,
								   UT_Bool bStart,
								   DG_DocMarkerId dmidParent,
								   UT_Bool bCreateEmptyRun,
								   DG_DocMarkerId * pdmidNew);
#endif 

	void draw(DG_Graphics*);
	void clearScreen(DG_Graphics*);

	void dump();

	/*
		Blocks are stored in a linked list which contains all of the blocks in
		the normal flow, in order.  A FL_BlockLayout knows in which FP_Column each
		of its slices is, but it does not know the FP_Column-relative Y coordinate
		of any slice.  Rather, it simply knows which FL_BlockLayout is right before it
		in the normal flow.
	*/

protected:
	void _addSlice(FP_BlockSlice*);
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
	FB_LineBreaker*			m_pBreaker;

	FL_BlockLayout*			m_pPrev;
	FL_BlockLayout*			m_pNext;

	FP_BlockSlice*			m_pCurrentSlice;
	FP_Run*					m_pFirstRun;
	FL_SectionLayout*		m_pSectionLayout;

	FP_Line*				m_pFirstLine;
	FP_Line*				m_pLastLine;

	int						m_bFormatting;
};

#endif /* BLOCKLAYOUT_H */
