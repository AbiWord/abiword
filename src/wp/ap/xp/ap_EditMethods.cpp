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

#define DLGHACK	
#ifdef DLGHACK			// see bottom of file for an apology
#ifdef WIN32
#include <windows.h>	// needs to be first
#endif
#ifdef LINUX
#include <gtk/gtk.h>
#endif
#endif /* DLGHACK */

#include <stdlib.h>
#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_EditMethod.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_Graphics.h"
#include "fl_Types.h"
#include "ap_App.h"
#include "ap_Frame.h"
#include "ap_EditMethods.h"


#ifdef DLGHACK
char * _promptFile(AP_Frame * pFrame, UT_Bool bSaveAs);
UT_Bool _chooseFont(AP_Frame * pFrame, FV_View * pView);
#endif /* DLGHACK */


/*****************************************************************/
/*****************************************************************/

/* abbreviations:
**   BOL    beginning of line
**   EOL    end of line
**   BOW    beginning of word
**   EOW    end of word
**   BOW    beginning of sentence
**   EOS    end of sentence
**   BOB    beginning of block
**   EOB    end of block
**   BOD    beginning of document
**   EOD    end of document
**
**   warpInsPt    warp insertion point
**   extSel       extend selection
**   del          delete
**   bck          backwards
**   fwd          forwards
**/

class ap_EditMethods
{
public:
	static EV_EditMethod_Fn scrollPageDown;
	static EV_EditMethod_Fn scrollPageUp;
	static EV_EditMethod_Fn scrollPageLeft;
	static EV_EditMethod_Fn scrollPageRight;
	static EV_EditMethod_Fn scrollLineDown;
	static EV_EditMethod_Fn scrollLineUp;
	static EV_EditMethod_Fn scrollLineLeft;
	static EV_EditMethod_Fn scrollLineRight;
	static EV_EditMethod_Fn scrollToTop;
	static EV_EditMethod_Fn scrollToBottom;

	static EV_EditMethod_Fn warpInsPtToXY;
	static EV_EditMethod_Fn warpInsPtLeft;
	static EV_EditMethod_Fn warpInsPtRight;
	static EV_EditMethod_Fn warpInsPtBOL;
	static EV_EditMethod_Fn warpInsPtEOL;
	static EV_EditMethod_Fn warpInsPtBOW;
	static EV_EditMethod_Fn warpInsPtEOW;
	static EV_EditMethod_Fn warpInsPtBOS;
	static EV_EditMethod_Fn warpInsPtEOS;
	static EV_EditMethod_Fn warpInsPtBOB;
	static EV_EditMethod_Fn warpInsPtEOB;
	static EV_EditMethod_Fn warpInsPtBOD;
	static EV_EditMethod_Fn warpInsPtEOD;
	static EV_EditMethod_Fn warpInsPtPrevLine;
	static EV_EditMethod_Fn warpInsPtNextLine;

	static EV_EditMethod_Fn extSelToXY;
	static EV_EditMethod_Fn extSelLeft;
	static EV_EditMethod_Fn extSelRight;
	static EV_EditMethod_Fn extSelBOL;
	static EV_EditMethod_Fn extSelEOL;
	static EV_EditMethod_Fn extSelBOW;
	static EV_EditMethod_Fn extSelEOW;
	static EV_EditMethod_Fn extSelBOS;
	static EV_EditMethod_Fn extSelEOS;
	static EV_EditMethod_Fn extSelBOB;
	static EV_EditMethod_Fn extSelEOB;
	static EV_EditMethod_Fn extSelBOD;
	static EV_EditMethod_Fn extSelEOD;
	static EV_EditMethod_Fn extSelPrevLine;
	static EV_EditMethod_Fn extSelNextLine;
	static EV_EditMethod_Fn extSelPageDown;
	static EV_EditMethod_Fn extSelPageUp;
	static EV_EditMethod_Fn selectAll;
	static EV_EditMethod_Fn selectWord;
	static EV_EditMethod_Fn selectLine;
	static EV_EditMethod_Fn selectBlock;
	
	static EV_EditMethod_Fn singleClick;
	static EV_EditMethod_Fn doubleClick;
	
	static EV_EditMethod_Fn delLeft;
	static EV_EditMethod_Fn delRight;
	static EV_EditMethod_Fn delBOL;
	static EV_EditMethod_Fn delEOL;
	static EV_EditMethod_Fn delBOW;
	static EV_EditMethod_Fn delEOW;
	static EV_EditMethod_Fn delBOS;
	static EV_EditMethod_Fn delEOS;
	static EV_EditMethod_Fn delBOB;
	static EV_EditMethod_Fn delEOB;
	static EV_EditMethod_Fn delBOD;
	static EV_EditMethod_Fn delEOD;

	static EV_EditMethod_Fn insertData;
	static EV_EditMethod_Fn insertTab;
	static EV_EditMethod_Fn insertSoftBreak;
	static EV_EditMethod_Fn insertParagraphBreak;
	static EV_EditMethod_Fn insertLineBreak;
	static EV_EditMethod_Fn insertPageBreak;
	static EV_EditMethod_Fn insertColumnBreak;

	static EV_EditMethod_Fn insertSpace;
	static EV_EditMethod_Fn insertNBSpace;
	
	static EV_EditMethod_Fn insFmtFaceTimes; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtFaceCourier; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtFaceArial; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize08; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize10; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize12; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize14; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize16; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize24; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtSize36; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtColorBlack; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtColorRed; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtColorGreen; // TODO we need a better way of doing this
	static EV_EditMethod_Fn insFmtColorBlue; // TODO we need a better way of doing this

	// TODO add functions for all of the standard menu commands.
	// TODO here are a few that i started.

	static EV_EditMethod_Fn fileNew;
	static EV_EditMethod_Fn fileOpen;
	static EV_EditMethod_Fn fileSave;
	static EV_EditMethod_Fn fileSaveAs;
	static EV_EditMethod_Fn print;

	static EV_EditMethod_Fn undo;
	static EV_EditMethod_Fn redo;
	static EV_EditMethod_Fn cut;
	static EV_EditMethod_Fn copy;
	static EV_EditMethod_Fn paste;

	static EV_EditMethod_Fn dlgFont;
	static EV_EditMethod_Fn toggleBold;
	static EV_EditMethod_Fn toggleItalic;
	static EV_EditMethod_Fn toggleUline;
	static EV_EditMethod_Fn toggleStrike;

	static EV_EditMethod_Fn alignLeft;
	static EV_EditMethod_Fn alignCenter;
	static EV_EditMethod_Fn alignRight;
	static EV_EditMethod_Fn alignJustify;

	static EV_EditMethod_Fn cycleWindows;
	static EV_EditMethod_Fn cycleWindowsBck;
	static EV_EditMethod_Fn closeWindow;
	static EV_EditMethod_Fn querySaveAndExit;

	// Test routines
	
	static EV_EditMethod_Fn Test_Dump;
};

/*****************************************************************/
/*****************************************************************/

#define _D_				EV_EMT_REQUIREDATA
#define _M_				EV_EMT_ALLOWMULTIPLIER
#define _DM_			EV_EMT_REQUIREDATA | EV_EMT_ALLOWMULTIPLIER

#define F(fn)			ap_EditMethods::fn
#define N(fn)			#fn
#define NF(fn)			N(fn), F(fn)

static EV_EditMethod s_arrayEditMethods[] = 
	{
		EV_EditMethod(NF(scrollPageDown),		_M_,	""),
		EV_EditMethod(NF(scrollPageUp),			_M_,	""),
		EV_EditMethod(NF(scrollPageLeft),		_M_,	""),
		EV_EditMethod(NF(scrollPageRight),		_M_,	""),
		EV_EditMethod(NF(scrollLineDown),		_M_,	""),
		EV_EditMethod(NF(scrollLineUp),			_M_,	""),
		EV_EditMethod(NF(scrollLineLeft),		_M_,	""),
		EV_EditMethod(NF(scrollLineRight),		_M_,	""),
		EV_EditMethod(NF(scrollToTop),			_M_,	""),
		EV_EditMethod(NF(scrollToBottom),		_M_,	""),

		EV_EditMethod(NF(warpInsPtToXY),		_M_,	""),
		EV_EditMethod(NF(warpInsPtLeft),		_M_,	""),
		EV_EditMethod(NF(warpInsPtRight),		_M_,	""),
		EV_EditMethod(NF(warpInsPtBOL),			_M_,	""),
		EV_EditMethod(NF(warpInsPtEOL),			_M_,	""),
		EV_EditMethod(NF(warpInsPtBOW),			_M_,	""),
		EV_EditMethod(NF(warpInsPtEOW),			_M_,	""),
		EV_EditMethod(NF(warpInsPtBOS),			_M_,	""),
		EV_EditMethod(NF(warpInsPtEOS),			_M_,	""),
		EV_EditMethod(NF(warpInsPtBOB),			_M_,	""),
		EV_EditMethod(NF(warpInsPtEOB),			_M_,	""),
		EV_EditMethod(NF(warpInsPtBOD),			_M_,	""),
		EV_EditMethod(NF(warpInsPtEOD),			_M_,	""),
		EV_EditMethod(NF(warpInsPtPrevLine),	_M_,	""),
		EV_EditMethod(NF(warpInsPtNextLine),	_M_,	""),

		EV_EditMethod(NF(extSelToXY),			_M_,	""),
		EV_EditMethod(NF(extSelLeft),			_M_,	""),
		EV_EditMethod(NF(extSelRight),			_M_,	""),
		EV_EditMethod(NF(extSelBOL),			_M_,	""),
		EV_EditMethod(NF(extSelEOL),			_M_,	""),
		EV_EditMethod(NF(extSelBOW),			_M_,	""),
		EV_EditMethod(NF(extSelEOW),			_M_,	""),
		EV_EditMethod(NF(extSelBOS),			_M_,	""),
		EV_EditMethod(NF(extSelEOS),			_M_,	""),
		EV_EditMethod(NF(extSelBOB),			_M_,	""),
		EV_EditMethod(NF(extSelEOB),			_M_,	""),
		EV_EditMethod(NF(extSelBOD),			_M_,	""),
		EV_EditMethod(NF(extSelEOD),			_M_,	""),
		EV_EditMethod(NF(extSelPrevLine),		_M_,	""),
		EV_EditMethod(NF(extSelNextLine),		_M_,	""),
		EV_EditMethod(NF(extSelPageDown),		_M_,	""),
		EV_EditMethod(NF(extSelPageUp),			_M_,	""),
		EV_EditMethod(NF(selectAll),			_M_,	""),
		EV_EditMethod(NF(selectWord),			_M_,	""),
		EV_EditMethod(NF(selectLine),			_M_,	""),
		EV_EditMethod(NF(selectBlock),			_M_,	""),

		EV_EditMethod(NF(singleClick),			_M_,	""),
		EV_EditMethod(NF(doubleClick),			_M_,	""),

		EV_EditMethod(NF(delLeft),				_M_,	""),
		EV_EditMethod(NF(delRight),				_M_,	""),
		EV_EditMethod(NF(delBOL),				_M_,	""),
		EV_EditMethod(NF(delEOL),				_M_,	""),
		EV_EditMethod(NF(delBOW),				_M_,	""),
		EV_EditMethod(NF(delEOW),				_M_,	""),
		EV_EditMethod(NF(delBOS),				_M_,	""),
		EV_EditMethod(NF(delEOS),				_M_,	""),
		EV_EditMethod(NF(delBOB),				_M_,	""),
		EV_EditMethod(NF(delEOB),				_M_,	""),
		EV_EditMethod(NF(delBOD),				_M_,	""),
		EV_EditMethod(NF(delEOD),				_M_,	""),

		EV_EditMethod(NF(insertData),			_DM_,	""),
		EV_EditMethod(NF(insertTab),			_M_,	""),
		EV_EditMethod(NF(insertSoftBreak),		_M_,	""),
		EV_EditMethod(NF(insertParagraphBreak),	_M_,	""),
		EV_EditMethod(NF(insertLineBreak),		_M_,	""),
		EV_EditMethod(NF(insertPageBreak),		_M_,	""),
		EV_EditMethod(NF(insertColumnBreak),	_M_,	""),

		EV_EditMethod(NF(insertSpace),			_M_,	""),
		EV_EditMethod(NF(insertNBSpace),		_M_,	""),

		EV_EditMethod(NF(insFmtFaceTimes),		_M_,	""),
		EV_EditMethod(NF(insFmtFaceCourier),	_M_,	""),
		EV_EditMethod(NF(insFmtFaceArial),		_M_,	""),
		EV_EditMethod(NF(insFmtSize08),			_M_,	""),
		EV_EditMethod(NF(insFmtSize10),			_M_,	""),
		EV_EditMethod(NF(insFmtSize12),			_M_,	""),
		EV_EditMethod(NF(insFmtSize14),			_M_,	""),
		EV_EditMethod(NF(insFmtSize16),			_M_,	""),
		EV_EditMethod(NF(insFmtSize24),			_M_,	""),
		EV_EditMethod(NF(insFmtSize36),			_M_,	""),
		EV_EditMethod(NF(insFmtColorBlack),		_M_,	""),
		EV_EditMethod(NF(insFmtColorRed),		_M_,	""),
		EV_EditMethod(NF(insFmtColorGreen),		_M_,	""),
		EV_EditMethod(NF(insFmtColorBlue),		_M_,	""),

		EV_EditMethod(NF(fileNew),				_M_,	""),
		EV_EditMethod(NF(fileOpen),				_M_,	""),
		EV_EditMethod(NF(fileSave),				_M_,	""),
		EV_EditMethod(NF(fileSaveAs),			_M_,	""),
		EV_EditMethod(NF(print),				_M_,	""),

		EV_EditMethod(NF(undo),					_M_,	""),
		EV_EditMethod(NF(redo),					_M_,	""),
		EV_EditMethod(NF(cut),					_M_,	""),
		EV_EditMethod(NF(copy),					_M_,	""),
		EV_EditMethod(NF(paste),				_M_,	""),

		EV_EditMethod(NF(dlgFont),				0,		""),
		EV_EditMethod(NF(toggleBold),			0,		""),
		EV_EditMethod(NF(toggleItalic),			0,		""),
		EV_EditMethod(NF(toggleUline),			0,		""),
		EV_EditMethod(NF(toggleStrike),			0,		""),

		EV_EditMethod(NF(alignLeft),			0,		""),
		EV_EditMethod(NF(alignCenter),			0,		""),
		EV_EditMethod(NF(alignRight),			0,		""),
		EV_EditMethod(NF(alignJustify),			0,		""),

		EV_EditMethod(NF(cycleWindows),			_M_,	""),
		EV_EditMethod(NF(cycleWindowsBck),		_M_,	""),
		EV_EditMethod(NF(closeWindow),			_M_,	""),
		EV_EditMethod(NF(querySaveAndExit),		_M_,	""),
		
		EV_EditMethod(NF(Test_Dump),			_M_,	"")
	};

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))


EV_EditMethodContainer * AP_GetEditMethods(void)
{
	// Construct a container for all of the methods this application
	// knows about.
	
	return new EV_EditMethodContainer(NrElements(s_arrayEditMethods),s_arrayEditMethods);
}

#undef _D_
#undef _M_
#undef _DM_
#undef F
#undef N
#undef NF

/*****************************************************************/
/*****************************************************************/

#define F(fn)		ap_EditMethods::fn
#define Defun(fn)	UT_Bool F(fn)(FV_View*   pView,   EV_EditMethodCallData *   pCallData  )
#define Defun0(fn)	UT_Bool F(fn)(FV_View* /*pView*/, EV_EditMethodCallData * /*pCallData*/)
#define Defun1(fn)	UT_Bool F(fn)(FV_View*   pView,   EV_EditMethodCallData * /*pCallData*/)
#define EX(fn)		F(fn)(pView, pCallData)


Defun1(scrollPageDown)
{
	pView->cmdScroll(DG_SCROLLCMD_PAGEDOWN);
	
	return UT_TRUE;
}

Defun1(scrollPageUp)
{
	pView->cmdScroll(DG_SCROLLCMD_PAGEUP);
	
	return UT_TRUE;
}

Defun1(scrollPageLeft)
{
	pView->cmdScroll(DG_SCROLLCMD_PAGELEFT);
	
	return UT_TRUE;
}

Defun1(scrollPageRight)
{
	pView->cmdScroll(DG_SCROLLCMD_PAGERIGHT);
	
	return UT_TRUE;
}

Defun1(scrollLineDown)
{
	pView->cmdScroll(DG_SCROLLCMD_LINEDOWN);
	
	return UT_TRUE;
}

Defun1(scrollLineUp)
{
	pView->cmdScroll(DG_SCROLLCMD_LINEUP);
	
	return UT_TRUE;
}

Defun1(scrollLineLeft)
{
	pView->cmdScroll(DG_SCROLLCMD_LINELEFT);
	
	return UT_TRUE;
}

Defun1(scrollLineRight)
{
	pView->cmdScroll(DG_SCROLLCMD_LINERIGHT);
	
	return UT_TRUE;
}

Defun1(scrollToTop)
{
	pView->cmdScroll(DG_SCROLLCMD_TOTOP);
	
	return UT_TRUE;
}

Defun1(scrollToBottom)
{
	pView->cmdScroll(DG_SCROLLCMD_TOBOTTOM);
	
	return UT_TRUE;
}

Defun(singleClick)
{
	UT_Bool bRes = UT_TRUE;

	// NOTE: context-free binding mechanism ==> we need this extra layer
	if (pView->isLeftMargin(pCallData->m_xPos, pCallData->m_yPos))
		bRes = EX(selectLine);
	else
		bRes = EX(warpInsPtToXY);
	
	return bRes;
}

Defun(doubleClick)
{
	UT_Bool bRes = UT_TRUE;

	// NOTE: context-free binding mechanism ==> we need this extra layer
	if (pView->isLeftMargin(pCallData->m_xPos, pCallData->m_yPos))
		bRes = EX(selectBlock);
	else
		bRes = EX(selectWord);
	
	return bRes;
}

Defun(warpInsPtToXY)
{
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos);
	
	return UT_TRUE;
}

Defun1(warpInsPtLeft)
{
	pView->cmdCharMotion(UT_FALSE,1);
	return UT_TRUE;
}

Defun1(warpInsPtRight)
{
	pView->cmdCharMotion(UT_TRUE,1);
	return UT_TRUE;
}

Defun1(warpInsPtBOL)
{
	pView->moveInsPtTo(FV_DOCPOS_BOL);
	return UT_TRUE;
}

Defun1(warpInsPtEOL)
{
	pView->moveInsPtTo(FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun1(warpInsPtBOW)
{
	pView->moveInsPtTo(FV_DOCPOS_BOW);
	return UT_TRUE;
}

Defun1(warpInsPtEOW)
{
	pView->moveInsPtTo(FV_DOCPOS_EOW);
	return UT_TRUE;
}

Defun0(warpInsPtBOS)
{
	return UT_TRUE;
}

Defun0(warpInsPtEOS)
{
	return UT_TRUE;
}

Defun1(warpInsPtBOB)
{
	pView->moveInsPtTo(FV_DOCPOS_BOB);
	return UT_TRUE;
}

Defun1(warpInsPtEOB)
{
	pView->moveInsPtTo(FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(warpInsPtBOD)
{
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	return UT_TRUE;
}

Defun1(warpInsPtEOD)
{
	pView->moveInsPtTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun1(warpInsPtPrevLine)
{
	pView->warpInsPtNextPrevLine(UT_FALSE);

	return UT_TRUE;
}

Defun1(warpInsPtNextLine)
{
	pView->warpInsPtNextPrevLine(UT_TRUE);

	return UT_TRUE;
}

Defun(extSelToXY)
{
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos);
	return UT_TRUE;
}
Defun1(extSelLeft)
{
	pView->extSelHorizontal(UT_FALSE,1);
	return UT_TRUE;
}

Defun1(extSelRight)
{
	pView->extSelHorizontal(UT_TRUE,1);
	return UT_TRUE;
}

Defun1(extSelBOL)
{
	pView->extSelTo(FV_DOCPOS_BOL);
	return UT_TRUE;
}

Defun1(extSelEOL)
{
	pView->extSelTo(FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun1(extSelBOW)
{
	pView->extSelTo(FV_DOCPOS_BOW);
	return UT_TRUE;
}

Defun1(extSelEOW)
{
	pView->extSelTo(FV_DOCPOS_EOW);
	return UT_TRUE;
}

Defun0(extSelBOS)
{
	return UT_TRUE;
}

Defun0(extSelEOS)
{
	return UT_TRUE;
}

Defun1(extSelBOB)
{
	pView->extSelTo(FV_DOCPOS_BOB);
	return UT_TRUE;
}

Defun1(extSelEOB)
{
	pView->extSelTo(FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(extSelBOD)
{
	pView->extSelTo(FV_DOCPOS_BOD);
	return UT_TRUE;
}

Defun1(extSelEOD)
{
	pView->extSelTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun1(extSelPrevLine)
{
	pView->extSelNextPrevLine(UT_FALSE);
	
	return UT_TRUE;
}

Defun1(extSelNextLine)
{
	pView->extSelNextPrevLine(UT_TRUE);
	
	return UT_TRUE;
}

Defun0(extSelPageDown)
{
	return UT_TRUE;
}

Defun0(extSelPageUp)
{
	return UT_TRUE;
}

Defun1(selectAll)
{
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	pView->extSelTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun(selectWord)
{
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOW, FV_DOCPOS_EOW);
	return UT_TRUE;
}

Defun(selectLine)
{
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOL, FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun(selectBlock)
{
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOB, FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(delLeft)
{
	pView->cmdCharDelete(UT_FALSE,1);
	return UT_TRUE;
}

Defun1(delRight)
{
	pView->cmdCharDelete(UT_TRUE,1);
	return UT_TRUE;
}

Defun1(delBOL)
{
	pView->delTo(FV_DOCPOS_BOL);
	return UT_TRUE;
}

Defun1(delEOL)
{
	pView->delTo(FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun1(delBOW)
{
	pView->delTo(FV_DOCPOS_BOW);
	return UT_TRUE;
}

Defun1(delEOW)
{
	pView->delTo(FV_DOCPOS_EOW);
	return UT_TRUE;
}

Defun0(delBOS)
{
	return UT_TRUE;
}

Defun0(delEOS)
{
	return UT_TRUE;
}

Defun1(delBOB)
{
	pView->delTo(FV_DOCPOS_BOB);
	return UT_TRUE;
}

Defun1(delEOB)
{
	pView->delTo(FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(delBOD)
{
	pView->delTo(FV_DOCPOS_BOD);
	return UT_TRUE;
}

Defun1(delEOD)
{
	pView->delTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun(insertData)
{
	pView->cmdCharInsert(pCallData->m_pData,pCallData->m_dataLength);
	return UT_TRUE;
}

Defun0(insertTab)
{
	return UT_TRUE;
}

Defun0(insertSoftBreak)
{
	return UT_TRUE;
}

Defun1(insertParagraphBreak)
{
	pView->insertParagraphBreak();
	
	return UT_TRUE;
}

Defun0(insertLineBreak)
{
	return UT_TRUE;
}

Defun0(insertPageBreak)
{
	return UT_TRUE;
}

Defun0(insertColumnBreak)
{
	return UT_TRUE;
}

Defun1(insertSpace)
{
	UT_UCSChar sp = 0x0020;
	pView->cmdCharInsert(&sp,1);
	return UT_TRUE;
}

Defun1(insertNBSpace)
{
	UT_UCSChar sp = 0x00a0;				// decimal 160 is NBS
	pView->cmdCharInsert(&sp,1);
	return UT_TRUE;
}

Defun1(insFmtFaceTimes)
{
	const XML_Char * properties[] =	{ "font-family", "Times New Roman", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtFaceCourier)
{
	const XML_Char * properties[] =	{ "font-family", "Courier New", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtFaceArial)
{
	const XML_Char * properties[] =	{ "font-family", "Arial", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize08)
{
	const XML_Char * properties[] =	{ "font-size", "8pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize10)
{
	const XML_Char * properties[] =	{ "font-size", "10pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize12)
{
	const XML_Char * properties[] =	{ "font-size", "12pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize14)
{
	const XML_Char * properties[] =	{ "font-size", "14pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize16)
{
	const XML_Char * properties[] =	{ "font-size", "16pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize24)
{
	const XML_Char * properties[] =	{ "font-size", "24pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtSize36)
{
	const XML_Char * properties[] =	{ "font-size", "36pt", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}

Defun1(insFmtColorBlack)
{
	const XML_Char * properties[] =	{ "color", "000000", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtColorRed)
{
	const XML_Char * properties[] =	{ "color", "ff0000", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtColorGreen)
{
	const XML_Char * properties[] =	{ "color", "00ff00", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}
Defun1(insFmtColorBlue)
{
	const XML_Char * properties[] =	{ "color", "0000ff", 0};
	pView->setCharFormat(properties);
	return UT_TRUE;
}

Defun1(fileNew)
{
	AP_Frame * pFrame = (AP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	return pFrame->loadDocument(NULL);
}

Defun1(fileOpen)
{
	AP_Frame * pFrame = (AP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);
	UT_Bool bRes = UT_TRUE;

#ifdef DLGHACK
	char * pNewFile = _promptFile(pFrame, UT_FALSE);
#else
	char * pNewFile = NULL;	
#endif /* DLGHACK */

	if (pNewFile)
	{
		UT_DEBUGMSG(("fileOpen: loading [%s]\n",pNewFile));
		bRes = pFrame->loadDocument(pNewFile);
		free(pNewFile);
	}

	return bRes;
}

Defun(fileSave)
{
	FL_DocLayout* pLayout = pView->getLayout();
	UT_ASSERT(pLayout);

	// can only save without prompting if filename already known
	if (!pLayout->getDocument()->getFilename())
		return EX(fileSaveAs);

	pView->cmdSave();

	return UT_TRUE;
}

Defun1(fileSaveAs)
{
	AP_Frame * pFrame = (AP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

#ifdef DLGHACK
	char * pNewFile = _promptFile(pFrame, UT_TRUE);
#else
	char * pNewFile = NULL;	
#endif /* DLGHACK */

	if (pNewFile)
	{
		UT_DEBUGMSG(("fileSaveAs: saving as [%s]\n",pNewFile));
		pView->cmdSaveAs(pNewFile);
		free(pNewFile);
	}

	return UT_TRUE;
}

Defun0(print)
{
	return UT_TRUE;
}

Defun1(undo)
{
	pView->cmdUndo(1);
	return UT_TRUE;
}

Defun1(redo)
{
	pView->cmdRedo(1);
	return UT_TRUE;
}

Defun0(cut)
{
	return UT_TRUE;
}

Defun0(copy)
{
	return UT_TRUE;
}

Defun0(paste)
{
	return UT_TRUE;
}

Defun0(cycleWindows)
{
	return UT_TRUE;
}

Defun0(cycleWindowsBck)
{
	return UT_TRUE;
}

Defun0(closeWindow)
{
	return UT_TRUE;
}

Defun0(querySaveAndExit)
{
	// TODO: does the querySave part go here, or in the window-specific shutdown?

	// for now, just try to do the exit part 
#ifdef DLGHACK
#ifdef WIN32
	PostQuitMessage (0);
#endif
#ifdef LINUX
	gtk_main_quit();	// what miguel uses
//	exit(0);			// what Andy had
#endif
#endif /* DLGHACK */

	return UT_TRUE;
}

Defun(dlgFont)
{
	AP_Frame * pFrame = (AP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

#ifdef DLGHACK
	return _chooseFont(pFrame, pView);
#else
	return UT_TRUE;
#endif /* DLGHACK */
}

static UT_Bool _toggleSpan(FV_View * pView, const XML_Char * prop, const XML_Char * vOn, const XML_Char * vOff)
{
	const XML_Char * props_out[] =	{ NULL, NULL, 0};

	// get current font info from pView
	const XML_Char ** props_in = NULL;
	const XML_Char * s;

	if (!pView->getCharFormat(&props_in))
		return UT_FALSE;

	props_out[0] = prop;
	props_out[1] = vOn;

	// TODO: some properties, espec. text-decoration, should *not* be mutually-exclusive
	s = UT_getAttribute(prop, props_in);
	if (s && (0 == UT_stricmp(s, vOn)))
		props_out[1] = vOff;
	
	free(props_in);

	// set it either way
	pView->setCharFormat(props_out);
	return UT_TRUE;
}

Defun1(toggleBold)
{
	return _toggleSpan(pView, "font-weight", "bold", "normal");
}

Defun1(toggleItalic)
{
	return _toggleSpan(pView, "font-style", "italic", "normal");
}

Defun1(toggleUline)
{
	return _toggleSpan(pView, "text-decoration", "underline", "none");
}

Defun1(toggleStrike)
{
	return _toggleSpan(pView, "text-decoration", "line-through", "none");
}

Defun1(alignLeft)
{
	const XML_Char * properties[] =	{ "text-align", "left", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(alignCenter)
{
	const XML_Char * properties[] =	{ "text-align", "center", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(alignRight)
{
	const XML_Char * properties[] =	{ "text-align", "right", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(alignJustify)
{
	const XML_Char * properties[] =	{ "text-align", "justify", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(Test_Dump)
{
	pView->Test_Dump();
	return UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

#ifdef DLGHACK

/*
	Having these platform-specific IFDEFs here is a gruesome HACK.
	The only excuse is that it makes things easier while we figure 
	out a better scheme for calling platform-specific dialogs from 
	XP code.
*/

/*****************************************************************/
/*****************************************************************/

#ifdef WIN32
#include "ap_Win32App.h"
#include "ap_Win32Frame.h"
#include "gr_Win32Graphics.h"

char * _promptFile(AP_Frame * pFrame, UT_Bool bSaveAs)
{
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pFrame);
	HWND hwnd = pWin32Frame->getTopLevelWindow();

	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for filename

	szFile[0] = 0;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;

	// display the appropriate dialog box. 
	if (bSaveAs)
	{
		if (GetSaveFileName(&ofn)==TRUE) 
			return strdup(szFile);
	}
	else
	{
		ofn.Flags |= OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn)==TRUE) 
			return strdup(szFile);
	}

	DWORD err = CommDlgExtendedError();
	UT_DEBUGMSG(("Didn't get a file: reason=0x%x\n", err));
	UT_ASSERT(!err);

	return NULL;
}

UT_Bool _chooseFont(AP_Frame * pFrame, FV_View * pView)
{
	/* 
		WARNING: any changes to this function should be closely coordinated 
		with the equivalent logic in Win32Graphics::FindFont()
	*/
	AP_Win32Frame * pWin32Frame = static_cast<AP_Win32Frame *>(pFrame);
	AP_Win32App * pWin32App = static_cast<AP_Win32App *>(pFrame->getApp());
	HWND hwnd = pWin32Frame->getTopLevelWindow();

	CHOOSEFONT cf;				// common dialog box structure
	LOGFONT lf;					// logical font structure

	// Initialize CHOOSEFONT
	memset(&cf, 0, sizeof(CHOOSEFONT));
	memset(&lf, 0, sizeof(lf));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hwnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
	cf.hInstance = pWin32App->getInstance();

	// get current font info from pView
	const XML_Char ** props_in = NULL;
	const XML_Char * s;
	DG_Graphics * pG = pView->getLayout()->getGraphics();

	if (!pView->getCharFormat(&props_in))
		return UT_FALSE;

	// TODO: what about generic family names?
	s = UT_getAttribute("font-family", props_in);
	if (s)
		strcpy(lf.lfFaceName, s);
	else
		cf.Flags |= CF_NOFACESEL;

	s = UT_getAttribute("font-size", props_in);
	if (s)
		lf.lfHeight = -(pG->convertDimension(s));
	else
		cf.Flags |= CF_NOSIZESEL;

	s = UT_getAttribute("font-weight", props_in);
	if (s && (0 == UT_stricmp(s, "bold")))
		lf.lfWeight = 700;

	s = UT_getAttribute("font-style", props_in);
	if (s && (0 == UT_stricmp(s, "italic")))
		lf.lfItalic = TRUE;

	s = UT_getAttribute("text-decoration", props_in);
	if (s)
	{
		// TODO: these should *not* be mutually exclusive
		if (0 == UT_stricmp(s, "underline"))
			lf.lfUnderline = TRUE;

		else if (0 == UT_stricmp(s, "line-through"))
			lf.lfStrikeOut = TRUE;
	}

	s = UT_getAttribute("color", props_in);
	if (s)
	{
		UT_RGBColor c;
		UT_parseColor(s, c);
		DWORD rgbCurrent;		// current text color

		rgbCurrent = RGB(c.m_red, c.m_grn, c.m_blu);

		cf.rgbColors = rgbCurrent;
	}

	free(props_in);

	// raise the dialog
	if (ChooseFont(&cf)==TRUE) 
	{
		int i = 0;

		// currently a maximum of six simultaneous properties
		const XML_Char * props_out[] = { 
			NULL, NULL, 
			NULL, NULL, 
			NULL, NULL, 
			NULL, NULL, 
			NULL, NULL, 
			NULL, NULL, 
			0 };

		if (lf.lfFaceName)
		{
			props_out[i] = "font-family";
			props_out[i+1] = lf.lfFaceName;
			i += 2;
		}

		char buf_size[5];

		if (cf.iPointSize)
		{
			sprintf(buf_size, "%dpt", cf.iPointSize/10);
			props_out[i] = "font-size";
			props_out[i+1] = buf_size;
			i += 2;
		}

		if (cf.nFontType & BOLD_FONTTYPE)
		{
			props_out[i] = "font-weight";
			props_out[i+1] ="bold";
			i += 2;
		}
		else if (cf.nFontType & REGULAR_FONTTYPE)
		{
			props_out[i] = "font-weight";
			props_out[i+1] ="normal";
			i += 2;
		}

		if (cf.nFontType & ITALIC_FONTTYPE)
		{
			props_out[i] = "font-style";
			props_out[i+1] ="italic";
			i += 2;
		}
		else
		{
			props_out[i] = "font-style";
			props_out[i+1] ="normal";
			i += 2;
		}

		// TODO: these should *not* be mutually exclusive
		if (lf.lfUnderline == TRUE)
		{
			props_out[i] = "text-decoration";
			props_out[i+1] ="underline";
			i += 2;
		}
		else if (lf.lfStrikeOut == TRUE)
		{
			props_out[i] = "text-decoration";
			props_out[i+1] ="line-through";
			i += 2;
		}
		else 
		{
			props_out[i] = "text-decoration";
			props_out[i+1] ="none";
			i += 2;
		}

		char buf_color[6];

		sprintf(buf_color, "%02x%02x%02x", GetRValue(cf.rgbColors), GetGValue(cf.rgbColors), GetBValue(cf.rgbColors)); 

		props_out[i] = "color";
		props_out[i+1] = buf_color;

		pView->setCharFormat(props_out);

		return UT_TRUE;
	}

	DWORD err = CommDlgExtendedError();
	UT_DEBUGMSG(("Didn't get a font: reason=0x%x\n", err));
	UT_ASSERT(!err);

	return UT_FALSE;
}

// TODO: figure out what can be shared here and move it up 
UT_Bool _printDoc(HWND hwnd)
{
	PRINTDLG pd;
	DOCINFO di;

	memset(&pd, 0, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hwnd;
	pd.Flags = PD_RETURNDC | PD_NOSELECTION | PD_PAGENUMS;

	if(!PrintDlg(&pd))
		return UT_FALSE;

	PD_Document * doc;	// HACK to fake out compiler. otherwise useless

	const char* pname = doc->getFilename();

	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = pname;
	di.lpszOutput = NULL;
	Win32Graphics* ppG = new Win32Graphics(pd.hDC, &di);

	FL_DocLayout* pDL = new FL_DocLayout(doc, ppG);
	pDL->formatAll();
			
	UT_uint32 iHeight = pDL->getHeight();
	FV_View* pV = new FV_View(NULL, pDL);	// TODO: fix first arg?

	dg_DrawArgs da;
	da.pG = NULL;
	da.width = pDL->getWidth();
	da.height = pDL->getHeight();

	ppG->startPrint();
	
//TODO allow page ranges
	int nPages = pDL->countPages();
	for(int i = 0; i < nPages; i++)
	{
		ppG->startPage(doc->getFilename(), i, TRUE, da.width,da.height);
		pV->draw(i, &da);
	}
	ppG->endPrint();

	// clean up
	delete ppG;
	// these next 2 cause some asserts...do I need to do these?
	//	delete pDL;
	//	delete pV;
	DeleteDC(pd.hDC);
	// free any memory allocated by StartDoc
	if(pd.hDevMode != NULL)
		GlobalFree(pd.hDevMode);
	if(pd.hDevNames != NULL)
		GlobalFree(pd.hDevNames);

return UT_TRUE;
}

#endif /* WIN32 */


/*****************************************************************/
/*****************************************************************/

#ifdef LINUX

static void set_ok (GtkWidget * /*widget*/, UT_Bool *dialog_result)
{
	*dialog_result = TRUE;
	gtk_main_quit();
}

char * _promptFile(AP_Frame * /*pFrame*/, UT_Bool bSaveAs)
{
	GtkFileSelection *pFS;
	UT_Bool accepted = FALSE;
	char * fileName = NULL;
	
	pFS = (GtkFileSelection *)gtk_file_selection_new(bSaveAs ? "Save file" : "Open File");

	/* Connect the signals for Ok and Cancel */
	gtk_signal_connect(GTK_OBJECT(pFS->ok_button), "clicked",
			    GTK_SIGNAL_FUNC(set_ok), &accepted);
	gtk_signal_connect(GTK_OBJECT(pFS->cancel_button), "clicked",
			    GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

	// Do we really want to position at the cursor?
	//gtk_window_position(GTK_WINDOW(pFS), GTK_WIN_POS_MOUSE);
	
	gtk_file_selection_hide_fileop_buttons(pFS);

	/* Run the dialog */
	gtk_widget_show(GTK_WIDGET(pFS));
	gtk_grab_add(GTK_WIDGET(pFS));
	gtk_main();

	if (accepted)
	{
		UT_cloneString(fileName, gtk_file_selection_get_filename(pFS));
	}

	gtk_widget_destroy (GTK_WIDGET(pFS));

	return fileName;
}

UT_Bool _chooseFont(AP_Frame * pFrame, FV_View * pView)
{
	GtkFontSelectionDialog * cf;

	UT_Bool accepted = FALSE;
	gchar * selectedFont = NULL;

	// move that title to resources
	cf = (GtkFontSelectionDialog *) gtk_font_selection_dialog_new("Font Selection");

	// TODO: get the view's current font to pass in to selection dialog
	
	/* Connect the signals for Ok and Cancel */
	gtk_signal_connect(GTK_OBJECT(cf->ok_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(set_ok),
					   &accepted);
	gtk_signal_connect(GTK_OBJECT(cf->cancel_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(gtk_main_quit),
					   NULL);
	// TIP: bind something (null at least) to "destroy" or you'll
	// never get out of gtk_main();
	gtk_signal_connect(GTK_OBJECT(cf),
					   "destroy",
					   GTK_SIGNAL_FUNC(NULL),
					   NULL);

	// Do we really want to position at the cursor?  
	//gtk_window_position(GTK_WINDOW(cf), GTK_WIN_POS_MOUSE);

	/* Run the dialog */
	gtk_widget_show(GTK_WIDGET(cf));
	gtk_grab_add(GTK_WIDGET(cf));
	gtk_main();

	if (accepted)
	{
		// should we grab a GdkFont* via gtk_font_selection_dialog_get_font()?
		// this depends on the view's set/query architecture
		selectedFont = gtk_font_selection_dialog_get_font_name(cf);
		
		printf("User wants to use font [%s].\n\n", selectedFont);
		
		// TODO: set the view's font through it's super-duper yet-to-be methods
	}

	gtk_widget_destroy (GTK_WIDGET(cf));

	return UT_TRUE;
}

#endif /* LINUX */

/*****************************************************************/
/*****************************************************************/

#endif /* DLGHACK */
