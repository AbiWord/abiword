 
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


#ifndef FV_VIEW_H
#define FV_VIEW_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"

class FL_DocLayout;
class fl_DocListener;
class fl_BlockLayout;
class fp_Page;
class fp_Run;
class PD_Document;
class DG_Graphics;

enum
{
	DG_SCROLLCMD_PAGEUP,
	DG_SCROLLCMD_PAGEDOWN,
	DG_SCROLLCMD_LINEUP,
	DG_SCROLLCMD_LINEDOWN,
	DG_SCROLLCMD_PAGERIGHT,
	DG_SCROLLCMD_PAGELEFT,
	DG_SCROLLCMD_LINERIGHT,
	DG_SCROLLCMD_LINELEFT,
	DG_SCROLLCMD_TOTOP,
	DG_SCROLLCMD_TOBOTTOM,
	DG_SCROLLCMD_TOPOSITION
};

class FV_View
{
	friend class fl_DocListener;

public:
	FV_View(FL_DocLayout*);
	void setXScrollOffset(UT_sint32);
	void setYScrollOffset(UT_sint32);
	void setWindowSize(UT_sint32, UT_sint32);
	void draw();
	void draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32);

	// TODO some of these functions should move into protected
	
	void moveInsPtToBOD();
	void getPageScreenOffsets(fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);
	void getPageYOffset(fp_Page* pPage, UT_sint32& yoff);
	void invertBetweenPositions(PT_DocPosition left, PT_DocPosition right);

	void insertParagraphBreak();
	void insertCharacterFormatting(const XML_Char * properties[]);

	void cmdScroll(UT_sint32 iScrollCmd, UT_uint32 iPos = 0);
	void addScrollListener(void (*pfn)(UT_sint32, UT_sint32));
	void removeScrollListener(void (*pfn)(UT_sint32, UT_sint32));
	void sendScrollEvent(UT_sint32 xoff, UT_sint32 yoff);

	void cmdFormatBlock(const XML_Char * properties[]);

// ----------------------
	void			cmdSelectWord(UT_sint32 xPos, UT_sint32 yPos);
	void			cmdCharMotion(UT_Bool bForward, UT_uint32 count);
	UT_Bool			cmdCharInsert(UT_UCSChar * text, UT_uint32 count);
	void			cmdCharDelete(UT_Bool bForward, UT_uint32 count);
	
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos);
	void		    moveInsPtToBOL(void);
	void		    moveInsPtToEOL(void);
	void			warpInsPtNextPrevLine(UT_Bool bNext);
	void			extSelHorizontal(UT_Bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos);
	void			extSelNextPrevLine(UT_Bool bNext);

	void			cmdUndo(UT_uint32 count);
	void			cmdRedo(UT_uint32 count);
	void			cmdSave(void);
	
	void			Test_Dump(void);	/* TODO remove this */
// ----------------------
	
protected:
	void			    _moveInsPtNextPrevLine(UT_Bool bNext);

	void 				_findPositionCoords(UT_uint32 pos,
											UT_Bool b,
											UT_uint32& x,
											UT_uint32& y,
											UT_uint32& height,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun);
	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos);

	UT_Bool				_isSelectionEmpty();
	void				_moveToSelectionEnd(UT_Bool bForward);
	void				_clearSelection(void);
	void				_eraseSelection(void);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(void);
	UT_Bool				_insertFormatPair(const XML_Char * szName, const XML_Char * properties[]);
	void 				_updateInsertionPoint();
	void 				_xorInsertionPoint();
	void 				_eraseInsertionPoint();
	void				_eraseSelectionOrInsertionPoint();
	void				_drawSelectionOrInsertionPoint();
	void				_xorSelection();
	void				_swapSelectionOrientation(void);

	// HACK: localize handling of insertion point logic
	UT_uint32			_getPoint(void);
	void				_setPoint(UT_uint32 pt);
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2);
	UT_Bool				_charMotion(UT_Bool bForward,UT_uint32 countChars);

	UT_Bool				_isPointAP(void);
	PT_AttrPropIndex	_getPointAP(void);
	void				_setPointAP(PT_AttrPropIndex indexAP);
	UT_Bool				_clearPointAP(UT_Bool bNotify);
	
	PT_DocPosition		m_iInsPoint;
	UT_uint32			m_xPoint;
	UT_uint32			m_yPoint;
	UT_uint32			m_iPointHeight;
	
	UT_Bool				m_bPointVisible;
	UT_Bool				m_bInsPointRight;

	UT_Bool				m_bPointAP;
	PT_AttrPropIndex	m_apPoint;
	
	UT_Bool				m_bSelectionVisible;
	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	DG_Graphics*		m_pG;
	UT_sint32			m_xScrollOffset;
	UT_sint32			m_yScrollOffset;
	UT_sint32			m_iWindowHeight;
	UT_sint32			m_iWindowWidth;

	PT_DocPosition		m_iSelectionAnchor;
	UT_Bool				m_bSelection;
	UT_Vector           m_scrollListeners;
};

#endif /* FV_View_H */
