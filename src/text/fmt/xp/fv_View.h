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



#ifndef FV_VIEW_H
#define FV_VIEW_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xav_View.h"
#include "xav_Listener.h"
#include "pt_Types.h"
#include "gr_DrawArgs.h"

class FL_DocLayout;
class fl_DocListener;
class fl_BlockLayout;
class fp_Page;
class fp_Run;
class PD_Document;
class DG_Graphics;
class UT_Timer;

typedef enum _FVDocPos
{
	FV_DOCPOS_BOB, FV_DOCPOS_EOB,	// block
	FV_DOCPOS_BOD, FV_DOCPOS_EOD,	// document
	FV_DOCPOS_BOL, FV_DOCPOS_EOL,	// line
	FV_DOCPOS_BOS, FV_DOCPOS_EOS,	// sentence
	FV_DOCPOS_BOW, FV_DOCPOS_EOW	// word
} FV_DocPos;

struct fv_ChangeState
{
	UT_Bool				bUndo;
	UT_Bool				bRedo;
	UT_Bool				bDirty;
	UT_Bool				bSelection;
	const XML_Char **	propsChar;
	const XML_Char **	propsBlock;
};

class FV_View : public AV_View
{
	friend class fl_DocListener;

public:
	FV_View(void*, FL_DocLayout*);
	~FV_View();

	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual void	draw(const UT_Rect* pRect=(UT_Rect*) NULL);

	virtual UT_Bool	notifyListeners(const AV_ChangeMask hint);

	virtual UT_Bool	canDo(UT_Bool bUndo) const;
	virtual void	cmdUndo(UT_uint32 count);
	virtual void	cmdRedo(UT_uint32 count);
	virtual void	cmdSave(void);
	virtual void	cmdSaveAs(const char * szFilename);

	virtual void	cmdCopy(void);
	virtual void	cmdCut(void);
	virtual void	cmdPaste(void);

// ----------------------
	FL_DocLayout* getLayout() const;

	void draw(int page, dg_DrawArgs* da);
	void draw(UT_sint32, UT_sint32, UT_sint32, UT_sint32, UT_Bool bClip=UT_FALSE);

	// TODO some of these functions should move into protected
	
	void getPageScreenOffsets(fp_Page* pPage, UT_sint32& xoff, UT_sint32& yoff, UT_sint32& width, UT_sint32& height);
	void getPageYOffset(fp_Page* pPage, UT_sint32& yoff);

	UT_Bool setBlockFormat(const XML_Char * properties[]);
	UT_Bool getBlockFormat(const XML_Char *** properties);

	UT_Bool setCharFormat(const XML_Char * properties[]);
	UT_Bool getCharFormat(const XML_Char *** properties);

	void insertParagraphBreak();

// ----------------------
	UT_Bool			isLeftMargin(UT_sint32 xPos, UT_sint32 yPos);
	UT_Bool			isSelectionEmpty();
	void			cmdSelect(UT_sint32 xPos, UT_sint32 yPos, FV_DocPos dpBeg, FV_DocPos dpEnd);
	void			cmdCharMotion(UT_Bool bForward, UT_uint32 count);
	UT_Bool			cmdCharInsert(UT_UCSChar * text, UT_uint32 count);
	void			cmdCharDelete(UT_Bool bForward, UT_uint32 count);
	void			delTo(FV_DocPos dp);

	// clipboard stuff, prototypes may change as we accomodate new clipboard data streams
	UT_Bool			pasteBlock(UT_UCSChar * text, UT_uint32 count);
	
	void			warpInsPtToXY(UT_sint32 xPos, UT_sint32 yPos);
	void			moveInsPtTo(FV_DocPos dp);
	void			warpInsPtNextPrevLine(UT_Bool bNext);
	void			extSelHorizontal(UT_Bool bForward, UT_uint32 count);
	void			extSelToXY(UT_sint32 xPos, UT_sint32 yPos, UT_Bool bDrag);
	void			extSelTo(FV_DocPos dp);
	void			extSelNextPrevLine(UT_Bool bNext);
	void			endDrag(UT_sint32 xPos, UT_sint32 yPos);
	
#if defined(PT_TEST) || defined(FMT_TEST)
	void			Test_Dump(void);
#endif
// ----------------------
	
protected:
	void				_drawBetweenPositions(PT_DocPosition left, PT_DocPosition right);
	UT_Bool				_ensureThatInsertionPointIsOnScreen(void);
	void			    _moveInsPtNextPrevLine(UT_Bool bNext);

	PT_DocPosition		_getDocPos(FV_DocPos dp, UT_Bool bKeepLooking=UT_TRUE);
	void 				_findPositionCoords(UT_uint32 pos,
											UT_Bool b,
											UT_sint32& x,
											UT_sint32& y,
											UT_uint32& height,
											fl_BlockLayout** ppBlock,
											fp_Run** ppRun);
	fl_BlockLayout* 	_findBlockAtPosition(PT_DocPosition pos);

	fp_Page*			_getPageForXY(UT_sint32 xPos, 
									  UT_sint32 yPos, 
									  UT_sint32& yClick);

	void				_moveToSelectionEnd(UT_Bool bForward);
	void				_clearSelection(void);
	void				_resetSelection(void);
	void				_setSelectionAnchor(void);
	void				_deleteSelection(void);
	UT_Bool				_insertFormatPair(const XML_Char * szName, const XML_Char * properties[]);
	void 				_updateInsertionPoint();
	void				_fixInsertionPointCoords();
	void 				_xorInsertionPoint();
	void 				_eraseInsertionPoint();
	void				_drawInsertionPoint();
	void				_drawSelection();
	void				_swapSelectionOrientation(void);
	void				_extSelToPos(PT_DocPosition pos);

	static void			_autoScroll(UT_Timer * pTimer);

	// localize handling of insertion point logic
	UT_uint32			_getPoint(void);
	void				_setPoint(UT_uint32 pt, UT_Bool bEOL = UT_FALSE);
	UT_uint32			_getDataCount(UT_uint32 pt1, UT_uint32 pt2);
	UT_Bool				_charMotion(UT_Bool bForward,UT_uint32 countChars);
	UT_Bool				_isPointAP(void);
	PT_AttrPropIndex	_getPointAP(void);
	void				_setPointAP(PT_AttrPropIndex indexAP);
	UT_Bool				_clearPointAP(UT_Bool bNotify);
	
	PT_DocPosition		m_iInsPoint;
	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	UT_uint32			m_iPointHeight;
	
	UT_Bool				m_bPointVisible;
	UT_Bool				m_bPointEOL;

	UT_Bool				m_bPointAP;
	PT_AttrPropIndex	m_apPoint;
	
	FL_DocLayout*		m_pLayout;
	PD_Document*		m_pDoc;
	DG_Graphics*		m_pG;

	PT_DocPosition		m_iSelectionAnchor;
	UT_Bool				m_bSelection;

	// autoscroll stuff
	UT_Timer *			m_pAutoScrollTimer;
	UT_sint32			m_xLastMouse;
	UT_sint32			m_yLastMouse;

	fv_ChangeState		m_chg;
};

#endif /* FV_VIEW_H */
