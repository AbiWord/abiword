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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_Graphics.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "ap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_MessageBox.h"
#include "xap_Dialog_FileOpenSaveAs.h"
#include "xap_Dialog_FontChooser.h"
#include "xap_Dialog_Print.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)

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

	// TODO add functions for all of the standard menu commands.
	// TODO here are a few that i started.

	static EV_EditMethod_Fn fileNew;
	static EV_EditMethod_Fn fileOpen;
	static EV_EditMethod_Fn fileSave;
	static EV_EditMethod_Fn fileSaveAs;
	static EV_EditMethod_Fn print;
	static EV_EditMethod_Fn printTB;

	static EV_EditMethod_Fn undo;
	static EV_EditMethod_Fn redo;
	static EV_EditMethod_Fn cut;
	static EV_EditMethod_Fn copy;
	static EV_EditMethod_Fn paste;
	static EV_EditMethod_Fn find;
	static EV_EditMethod_Fn go;
	static EV_EditMethod_Fn replace;

	static EV_EditMethod_Fn dlgFont;
	static EV_EditMethod_Fn fontFamily;
	static EV_EditMethod_Fn fontSize;
	static EV_EditMethod_Fn toggleBold;
	static EV_EditMethod_Fn toggleItalic;
	static EV_EditMethod_Fn toggleUline;
	static EV_EditMethod_Fn toggleStrike;
	static EV_EditMethod_Fn togglePlain;

	static EV_EditMethod_Fn alignLeft;
	static EV_EditMethod_Fn alignCenter;
	static EV_EditMethod_Fn alignRight;
	static EV_EditMethod_Fn alignJustify;

	static EV_EditMethod_Fn singleSpace;
	static EV_EditMethod_Fn middleSpace;
	static EV_EditMethod_Fn doubleSpace;

	static EV_EditMethod_Fn activateWindow_1;
	static EV_EditMethod_Fn activateWindow_2;
	static EV_EditMethod_Fn activateWindow_3;
	static EV_EditMethod_Fn activateWindow_4;
	static EV_EditMethod_Fn activateWindow_5;
	static EV_EditMethod_Fn activateWindow_6;
	static EV_EditMethod_Fn activateWindow_7;
	static EV_EditMethod_Fn activateWindow_8;
	static EV_EditMethod_Fn activateWindow_9;
	static EV_EditMethod_Fn moreWindowsDlg;

	static EV_EditMethod_Fn newWindow;
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

	EV_EditMethod(NF(fileNew),				_M_,	""),
	EV_EditMethod(NF(fileOpen),				_M_,	""),
	EV_EditMethod(NF(fileSave),				_M_,	""),
	EV_EditMethod(NF(fileSaveAs),			_M_,	""),
	EV_EditMethod(NF(print),				_M_,	""),
	EV_EditMethod(NF(printTB),				_M_,	""), // avoid query if possible

	EV_EditMethod(NF(undo),					_M_,	""),
	EV_EditMethod(NF(redo),					_M_,	""),
	EV_EditMethod(NF(cut),					_M_,	""),
	EV_EditMethod(NF(copy),					_M_,	""),
	EV_EditMethod(NF(paste),				_M_,	""),
	EV_EditMethod(NF(find),					_M_,	""),
	EV_EditMethod(NF(go),					_M_,	""),
	EV_EditMethod(NF(replace),				_M_,	""),

	EV_EditMethod(NF(dlgFont),				0,		""),
	EV_EditMethod(NF(fontFamily),			_D_,	""),
	EV_EditMethod(NF(fontSize),				_D_,	""),
	EV_EditMethod(NF(toggleBold),			0,		""),
	EV_EditMethod(NF(toggleItalic),			0,		""),
	EV_EditMethod(NF(toggleUline),			0,		""),
	EV_EditMethod(NF(toggleStrike),			0,		""),
	EV_EditMethod(NF(togglePlain),			0,		""),

	EV_EditMethod(NF(alignLeft),			0,		""),
	EV_EditMethod(NF(alignCenter),			0,		""),
	EV_EditMethod(NF(alignRight),			0,		""),
	EV_EditMethod(NF(alignJustify),			0,		""),

	EV_EditMethod(NF(singleSpace),			0,		""),
	EV_EditMethod(NF(middleSpace),			0,		""),
	EV_EditMethod(NF(doubleSpace),			0,		""),

	EV_EditMethod(NF(activateWindow_1),		0,		""),
	EV_EditMethod(NF(activateWindow_2),		0,		""),
	EV_EditMethod(NF(activateWindow_3),		0,		""),
	EV_EditMethod(NF(activateWindow_4),		0,		""),
	EV_EditMethod(NF(activateWindow_5),		0,		""),
	EV_EditMethod(NF(activateWindow_6),		0,		""),
	EV_EditMethod(NF(activateWindow_7),		0,		""),
	EV_EditMethod(NF(activateWindow_8),		0,		""),
	EV_EditMethod(NF(activateWindow_9),		0,		""),
	EV_EditMethod(NF(moreWindowsDlg),		0,		""),

	EV_EditMethod(NF(newWindow),			_M_,	""),
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
#define Defun(fn)	UT_Bool F(fn)(AV_View*   pAV_View,   EV_EditMethodCallData *   pCallData  )
#define Defun0(fn)	UT_Bool F(fn)(AV_View* /*pAV_View*/, EV_EditMethodCallData * /*pCallData*/)
#define Defun1(fn)	UT_Bool F(fn)(AV_View*   pAV_View,   EV_EditMethodCallData * /*pCallData*/)
#define EX(fn)		F(fn)(pAV_View, pCallData)


Defun1(scrollPageDown)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEDOWN);

	return UT_TRUE;
}

Defun1(scrollPageUp)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGEUP);

	return UT_TRUE;
}

Defun1(scrollPageLeft)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGELEFT);

	return UT_TRUE;
}

Defun1(scrollPageRight)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_PAGERIGHT);

	return UT_TRUE;
}

Defun1(scrollLineDown)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEDOWN);

	return UT_TRUE;
}

Defun1(scrollLineUp)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINEUP);

	return UT_TRUE;
}

Defun1(scrollLineLeft)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINELEFT);

	return UT_TRUE;
}

Defun1(scrollLineRight)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_LINERIGHT);

	return UT_TRUE;
}

Defun1(scrollToTop)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_TOTOP);

	return UT_TRUE;
}

Defun1(scrollToBottom)
{
	pAV_View->cmdScroll(AV_SCROLLCMD_TOBOTTOM);

	return UT_TRUE;
}

Defun1(fileNew)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	AP_Frame * pNewFrame = pApp->newFrame();

	if (pNewFrame)
		pFrame = pNewFrame;

	UT_Bool bRet = pFrame->loadDocument(NULL);

	if (pNewFrame)
		pNewFrame->show();

	return bRet;
}

/*****************************************************************/
/*****************************************************************/

// TODO i've pulled the code to compose a question in a message
// TODO box into these little s_Ask*() functions.  part of this
// TODO is to isolate the question asking from the code which
// TODO decides what to do with the answer.  but also to see if
// TODO we want to abstract things further and make us think about
// TODO localization of the question strings....

static UT_Bool s_AskRevertFile(AP_Frame * pFrame)
{
	// return UT_TRUE if we should revert the file (back to the saved copy).

	pFrame->raise();

	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage("Revert to saved copy of %s?", pFrame->getFilename());
	pDialog->setButtons(AP_Dialog_MessageBox::b_YN);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_YES);

	pDialog->runModal(pFrame);

	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return (ans == AP_Dialog_MessageBox::a_YES);
}

static UT_Bool s_AskCloseAllAndExit(AP_Frame * pFrame)
{
	// return UT_TRUE if we should quit.

	pFrame->raise();

	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage("Close all windows and exit?");
	pDialog->setButtons(AP_Dialog_MessageBox::b_YN);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_NO);

	pDialog->runModal(pFrame);

	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return (ans == AP_Dialog_MessageBox::a_YES);
}

static AP_Dialog_MessageBox::tAnswer s_AskSaveFile(AP_Frame * pFrame)
{
	pFrame->raise();

	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage("Save changes to %s?", pFrame->getTitle(200));
	pDialog->setButtons(AP_Dialog_MessageBox::b_YNC);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_YES);

	pDialog->runModal(pFrame);

	AP_Dialog_MessageBox::tAnswer ans = pDialog->getAnswer();

	pDialogFactory->releaseDialog(pDialog);

	return (ans);
}

static UT_Bool s_AskForPathname(AP_Frame * pFrame,
								UT_Bool bSaveAs,
								const char * pSuggestedName,
								char ** ppPathname)
{
	// raise the file-open or file-save-as dialog.
	// return a_OK or a_CANCEL depending on which button
	// the user hits.
	// return a pointer a strdup()'d string containing the
	// pathname the user entered -- ownership of this goes
	// to the caller (so free it when you're done with it).

	UT_DEBUGMSG(("s_AskForPathname: frame %p, bSaveAs %ld, suggest=[%s]\n",
				 pFrame,bSaveAs,((pSuggestedName) ? pSuggestedName : "")));

	UT_ASSERT(ppPathname);
	*ppPathname = NULL;

	pFrame->raise();

	AP_Dialog_Id id = ((bSaveAs) ? XAP_DIALOG_ID_FILE_SAVEAS : XAP_DIALOG_ID_FILE_OPEN);

	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_FileOpenSaveAs * pDialog
		= (AP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	if (pSuggestedName && *pSuggestedName)
	{
		// if caller wants to suggest a name, use it and seed the
		// dialog in that directory and set the filename.
		pDialog->setCurrentPathname(pSuggestedName);
		pDialog->setSuggestFilename(UT_TRUE);
	}
	else
	{
		// if caller does not want to suggest a name, seed the dialog
		// to the directory containing this document (if it has a
		// name), but don't put anything in the filename portion.
		pDialog->setCurrentPathname(pFrame->getFilename());
		pDialog->setSuggestFilename(UT_FALSE);
	}

	pDialog->runModal(pFrame);

	AP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	UT_Bool bOK = (ans == AP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		if (szResultPathname && *szResultPathname)
			*ppPathname = strdup(szResultPathname);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*****************************************************************/
/*****************************************************************/

Defun1(fileOpen)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	UT_Bool bRes = UT_TRUE;

	char * pNewFile = NULL;
	UT_Bool bOK = s_AskForPathname(pFrame,UT_FALSE,NULL,&pNewFile);

	if (!bOK || !pNewFile)
		return UT_FALSE;

	// we own storage for pNewFile and must free it.

	UT_DEBUGMSG(("fileOpen: loading [%s]\n",pNewFile));
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);
	AP_Frame * pNewFrame = NULL;

	// see if requested file is already open in another frame
	UT_sint32 ndx = pApp->findFrame(pNewFile);

	if (ndx >= 0)
	{
		// yep, reuse it
		pNewFrame = pApp->getFrame(ndx);
		UT_ASSERT(pNewFrame);

		if (!s_AskRevertFile(pNewFrame))
		{
			// never mind
			free(pNewFile);
			return UT_FALSE;
		}
	}
	else if (pFrame->isDirty() || pFrame->getFilename() || (pFrame->getViewNumber() > 0))
	{
		/*
		  We generally open documents in a new frame, which keeps the
		  contents of the current frame available.

		  However, as a convenience we do replace the contents of the
		  current frame if it's the only top-level view on an empty,
		  untitled document.
		*/
		pNewFrame = pApp->newFrame();
		UT_ASSERT(pNewFrame);
	}

	if (pNewFrame)
		pFrame = pNewFrame;

	bRes = pFrame->loadDocument(pNewFile);

	// HACK: at least make something show
	if (!bRes)
		bRes = pFrame->loadDocument(NULL);

	if (pNewFrame)
		pNewFrame->show();

	free(pNewFile);

	return bRes;
}

Defun(fileSave)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);

	// can only save without prompting if filename already known
	if (!pFrame->getFilename())
		return EX(fileSaveAs);

	pAV_View->cmdSave();

	if (pFrame->getViewNumber() > 0)
	{
		AP_App * pApp = pFrame->getApp();
		UT_ASSERT(pApp);

		pApp->updateClones(pFrame);
	}

	return UT_TRUE;
}

Defun1(fileSaveAs)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);

	char * pNewFile = NULL;
	UT_Bool bOK = s_AskForPathname(pFrame,UT_TRUE,NULL,&pNewFile);

	if (!bOK || !pNewFile)
		return UT_FALSE;

	UT_DEBUGMSG(("fileSaveAs: saving as [%s]\n",pNewFile));
	pAV_View->cmdSaveAs(pNewFile);
	free(pNewFile);

	if (pFrame->getViewNumber() > 0)
	{
		AP_App * pApp = pFrame->getApp();
		UT_ASSERT(pApp);

		pApp->updateClones(pFrame);
	}

	return UT_TRUE;
}

Defun1(undo)
{
	pAV_View->cmdUndo(1);
	return UT_TRUE;
}

Defun1(redo)
{
	pAV_View->cmdRedo(1);
	return UT_TRUE;
}

Defun1(newWindow)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);

	return (pFrame->cloneFrame() ? UT_TRUE : UT_FALSE);
}

static UT_Bool _activateWindow(AV_View* pAV_View, UT_uint32 ndx)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	UT_ASSERT(ndx > 0);
	UT_ASSERT(ndx <= pApp->getFrameCount());

	AP_Frame * pSelFrame = pApp->getFrame(ndx - 1);

	if (pSelFrame)
		pSelFrame->raise();

	return UT_TRUE;
}

Defun1(activateWindow_1)
{
	return _activateWindow(pAV_View, 1);
}
Defun1(activateWindow_2)
{
	return _activateWindow(pAV_View, 2);
}
Defun1(activateWindow_3)
{
	return _activateWindow(pAV_View, 3);
}
Defun1(activateWindow_4)
{
	return _activateWindow(pAV_View, 4);
}
Defun1(activateWindow_5)
{
	return _activateWindow(pAV_View, 5);
}
Defun1(activateWindow_6)
{
	return _activateWindow(pAV_View, 6);
}
Defun1(activateWindow_7)
{
	return _activateWindow(pAV_View, 7);
}
Defun1(activateWindow_8)
{
	return _activateWindow(pAV_View, 8);
}
Defun1(activateWindow_9)
{
	return _activateWindow(pAV_View, 9);
}
Defun0(moreWindowsDlg)
{
	return UT_TRUE;
}

Defun1(cycleWindows)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	UT_sint32 ndx = pApp->findFrame(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx < (UT_sint32) pApp->getFrameCount() - 1)
		ndx++;
	else
		ndx = 0;

	AP_Frame * pSelFrame = pApp->getFrame(ndx);

	if (pSelFrame)
		pSelFrame->raise();

	return UT_TRUE;
}

Defun1(cycleWindowsBck)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	UT_sint32 ndx = pApp->findFrame(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx > 0)
		ndx--;
	else
		ndx = pApp->getFrameCount() - 1;

	AP_Frame * pSelFrame = pApp->getFrame(ndx);

	if (pSelFrame)
		pSelFrame->raise();

	return UT_TRUE;
}

Defun(closeWindow)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	// is this the last view on a dirty document?
	if ((pFrame->getViewNumber() == 0) &&
		(pFrame->isDirty()))
	{
		AP_Dialog_MessageBox::tAnswer ans = s_AskSaveFile(pFrame);

		switch (ans)
		{
		case AP_Dialog_MessageBox::a_YES:				// save it first
			{
				UT_Bool bRet = EX(fileSave);
				if (!bRet)								// didn't successfully save,
					return UT_FALSE;					//    so don't close
			}
			break;

		case AP_Dialog_MessageBox::a_NO:				// just close it
			break;

		case AP_Dialog_MessageBox::a_CANCEL:			// don't close it
			return UT_FALSE;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return UT_FALSE;
		}
	}

	// are we the last window?
	if (1 >= pApp->getFrameCount())
	{
		pApp->reallyExit();
	}

	// nuke the window
	pFrame->close();
	pApp->forgetFrame(pFrame);
	delete pFrame;

	return UT_TRUE;
}

Defun(querySaveAndExit)
{
	AP_Frame * pFrame = (AP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	AP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

	if (1 < pApp->getFrameCount())
	{
		if (!s_AskCloseAllAndExit(pFrame))
		{
			// never mind
			return UT_FALSE;
		}
	}

	UT_Bool bRet = UT_TRUE;
	UT_uint32 ndx = pApp->getFrameCount();

	// loop over windows, but stop if one can't close
	while (bRet && ndx > 0)
	{
		AP_Frame * f = pApp->getFrame(ndx - 1);
		UT_ASSERT(f);
		pAV_View = f->getCurrentView();
		UT_ASSERT(pAV_View);

		bRet = EX(closeWindow);

		ndx--;
	}

	if (bRet)
	{
		// TODO: this shouldn't be necessary, but just in case
		pApp->reallyExit();
	}

	return bRet;
}

/*****************************************************************/
/*****************************************************************/

#define ABIWORD_VIEW  	FV_View * pView = static_cast<FV_View *>(pAV_View)


Defun(singleClick)
{
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
	pView->warpInsPtToXY(pCallData->m_xPos, pCallData->m_yPos);

	return UT_TRUE;
}

Defun1(warpInsPtLeft)
{
	ABIWORD_VIEW;
	pView->cmdCharMotion(UT_FALSE,1);
	return UT_TRUE;
}

Defun1(warpInsPtRight)
{
	ABIWORD_VIEW;
	pView->cmdCharMotion(UT_TRUE,1);
	return UT_TRUE;
}

Defun1(warpInsPtBOL)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOL);
	return UT_TRUE;
}

Defun1(warpInsPtEOL)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun1(warpInsPtBOW)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOW);
	return UT_TRUE;
}

Defun1(warpInsPtEOW)
{
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOB);
	return UT_TRUE;
}

Defun1(warpInsPtEOB)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(warpInsPtBOD)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	return UT_TRUE;
}

Defun1(warpInsPtEOD)
{
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun1(warpInsPtPrevLine)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevLine(UT_FALSE);
	return UT_TRUE;
}

Defun1(warpInsPtNextLine)
{
	ABIWORD_VIEW;
	pView->warpInsPtNextPrevLine(UT_TRUE);
	return UT_TRUE;
}

Defun(extSelToXY)
{
	ABIWORD_VIEW;
	pView->extSelToXY(pCallData->m_xPos, pCallData->m_yPos);
	return UT_TRUE;
}
Defun1(extSelLeft)
{
	ABIWORD_VIEW;
	pView->extSelHorizontal(UT_FALSE,1);
	return UT_TRUE;
}

Defun1(extSelRight)
{
	ABIWORD_VIEW;
	pView->extSelHorizontal(UT_TRUE,1);
	return UT_TRUE;
}

Defun1(extSelBOL)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOL);
	return UT_TRUE;
}

Defun1(extSelEOL)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun1(extSelBOW)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOW);
	return UT_TRUE;
}

Defun1(extSelEOW)
{
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOB);
	return UT_TRUE;
}

Defun1(extSelEOB)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(extSelBOD)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_BOD);
	return UT_TRUE;
}

Defun1(extSelEOD)
{
	ABIWORD_VIEW;
	pView->extSelTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun1(extSelPrevLine)
{
	ABIWORD_VIEW;
	pView->extSelNextPrevLine(UT_FALSE);
	return UT_TRUE;
}

Defun1(extSelNextLine)
{
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
	pView->moveInsPtTo(FV_DOCPOS_BOD);
	pView->extSelTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun(selectWord)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOW, FV_DOCPOS_EOW);
	return UT_TRUE;
}

Defun(selectLine)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOL, FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun(selectBlock)
{
	ABIWORD_VIEW;
	pView->cmdSelect(pCallData->m_xPos, pCallData->m_yPos, FV_DOCPOS_BOB, FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(delLeft)
{
	ABIWORD_VIEW;
	pView->cmdCharDelete(UT_FALSE,1);
	return UT_TRUE;
}

Defun1(delRight)
{
	ABIWORD_VIEW;
	pView->cmdCharDelete(UT_TRUE,1);
	return UT_TRUE;
}

Defun1(delBOL)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOL);
	return UT_TRUE;
}

Defun1(delEOL)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOL);
	return UT_TRUE;
}

Defun1(delBOW)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOW);
	return UT_TRUE;
}

Defun1(delEOW)
{
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOB);
	return UT_TRUE;
}

Defun1(delEOB)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOB);
	return UT_TRUE;
}

Defun1(delBOD)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_BOD);
	return UT_TRUE;
}

Defun1(delEOD)
{
	ABIWORD_VIEW;
	pView->delTo(FV_DOCPOS_EOD);
	return UT_TRUE;
}

Defun(insertData)
{
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
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
	ABIWORD_VIEW;
	UT_UCSChar sp = 0x0020;
	pView->cmdCharInsert(&sp,1);
	return UT_TRUE;
}

Defun1(insertNBSpace)
{
	ABIWORD_VIEW;
	UT_UCSChar sp = 0x00a0;				// decimal 160 is NBS
	pView->cmdCharInsert(&sp,1);
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

Defun0(find)
{
	return UT_TRUE;
}

Defun0(go)
{
	return UT_TRUE;
}

Defun0(replace)
{
	return UT_TRUE;
}

/*****************************************************************/

static UT_Bool s_doFontDlg(FV_View * pView)
{
	AP_Frame * pFrame = (AP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	AP_Dialog_Id id = XAP_DIALOG_ID_FONT;

	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_FontChooser * pDialog
		= (AP_Dialog_FontChooser *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// stuff the DG_Graphics into the dialog so that it
	// can query the system for font info relative to our
	// context.

	pDialog->setGraphicsContext(pView->getLayout()->getGraphics());

	// get current font info from pView

	const XML_Char ** props_in = NULL;
	if (pView->getCharFormat(&props_in))
	{
		// stuff font properties into the dialog.
		// for a/p which are constant across the selection (always
		// present) we will set the field in the dialog.  for things
		// which change across the selection, we ask the dialog not
		// to set the field (by passing null).

		pDialog->setFontFamily(UT_getAttribute("font-family", props_in));
		pDialog->setFontSize(UT_getAttribute("font-size", props_in));
		pDialog->setFontWeight(UT_getAttribute("font-weight", props_in));
		pDialog->setFontStyle(UT_getAttribute("font-style", props_in));
		pDialog->setColor(UT_getAttribute("color", props_in));

		// these behave a little differently since they are
		// probably just check boxes and we don't have to
		// worry about initializing a combo box with a choice
		// (and because they are all stuck under one CSS attribute).

		const XML_Char * s = UT_getAttribute("text-decoration", props_in);
		UT_Bool bUnderline = (strstr(s, "underline") != NULL);
		UT_Bool bStrikeOut = (strstr(s, "line-through") != NULL);
		pDialog->setFontDecoration(bUnderline,bStrikeOut);

		free(props_in);
	}

	// run the dialog

	pDialog->runModal(pFrame);

	// extract what they did

	UT_Bool bOK = (pDialog->getAnswer() == AP_Dialog_FontChooser::a_OK);

	if (bOK)
	{
		UT_uint32  k = 0;
		const XML_Char * props_out[17];
		const XML_Char * s;

		if (pDialog->getChangedFontFamily(&s))
		{
			props_out[k++] = "font-family";
			props_out[k++] = s;
		}

		if (pDialog->getChangedFontSize(&s))
		{
			props_out[k++] = "font-size";
			props_out[k++] = s;
		}

		if (pDialog->getChangedFontWeight(&s))
		{
			props_out[k++] = "font-weight";
			props_out[k++] = s;
		}

		if (pDialog->getChangedFontStyle(&s))
		{
			props_out[k++] = "font-style";
			props_out[k++] = s;
		}

		if (pDialog->getChangedColor(&s))
		{
			props_out[k++] = "color";
			props_out[k++] = s;
		}

		UT_Bool bUnderline = UT_FALSE;
		UT_Bool bChangedUnderline = pDialog->getChangedUnderline(&bUnderline);
		UT_Bool bStrikeOut = UT_FALSE;
		UT_Bool bChangedStrikeOut = pDialog->getChangedStrikeOut(&bStrikeOut);

		if (bChangedUnderline || bChangedStrikeOut)
		{
			if (bUnderline && bStrikeOut)
				s = "underline line-through";
			else if (bUnderline)
				s = "underline";
			else if (bStrikeOut)
				s = "line-through";
			else
				s = "none";

			props_out[k++] = "text-decoration";
			props_out[k++] = s;
		}

		props_out[k] = 0;						// put null after last pair.
		UT_ASSERT(k < NrElements(props_out));

		if (k > 0)								// if something changed
			pView->setCharFormat(props_out);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*****************************************************************/

Defun1(dlgFont)
{
	ABIWORD_VIEW;

	return s_doFontDlg(pView);
}

Defun(fontFamily)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "font-family", NULL, 0};
	properties[1] = (const XML_Char *) pCallData->m_pData;
	pView->setCharFormat(properties);
	return UT_TRUE;
}

Defun(fontSize)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "font-size", NULL, 0};

	// BUGBUG: stupid casting trick will eventually bite us
	const XML_Char * sz = (const XML_Char *) pCallData->m_pData;

	if (sz && *sz)
	{
		int len = strlen(sz) + 2;
		XML_Char * buf = (XML_Char *) calloc(len, sizeof(XML_Char *));

		sprintf(buf, "%spt", sz);

		properties[1] = buf;
		pView->setCharFormat(properties);

		FREEP(buf);
	}
	return UT_TRUE;
}

/*****************************************************************/

static UT_Bool _toggleSpan(FV_View * pView,
						   const XML_Char * prop,
						   const XML_Char * vOn,
						   const XML_Char * vOff,
						   UT_Bool bMultiple=UT_FALSE)
{
	const XML_Char * props_out[] =	{ NULL, NULL, 0};

	// get current font info from pView
	const XML_Char ** props_in = NULL;
	const XML_Char * s;

	if (!pView->getCharFormat(&props_in))
		return UT_FALSE;

	props_out[0] = prop;
	props_out[1] = vOn;		// be optimistic

	XML_Char * buf = NULL;

	s = UT_getAttribute(prop, props_in);
	if (s)
	{
		if (bMultiple)
		{
			// some properties have multiple values
			XML_Char*	p = strstr(s, vOn);

			if (p)
			{
				// yep...
				if (strstr(s, vOff))
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

				// ... take it out
				int len = strlen(s);
				buf = (XML_Char *) calloc(len, sizeof(XML_Char *));

				strncpy(buf, s, p - s);
				strcat(buf, s + (p - s) + strlen(vOn));

				// now see if anything's left
				XML_Char * q;
				UT_cloneString(q, buf);

				if (q && strtok(q, " "))
					props_out[1] = buf;		// yep, use it
				else
					props_out[1] = vOff;	// nope, clear it

				free(q);
			}
			else
			{
				// nope...
				if (UT_stricmp(s, vOff))
				{
					// ...put it in by appending to current contents
					int len = strlen(s) + strlen(vOn) + 2;
					buf = (XML_Char *) calloc(len, sizeof(XML_Char *));

					strcpy(buf, s);
					strcat(buf, " ");
					strcat(buf, vOn);

					props_out[1] = buf;
				}
			}
		}
		else
		{
			if (0 == UT_stricmp(s, vOn))
				props_out[1] = vOff;
		}
	}
	free(props_in);

	// set it either way
	pView->setCharFormat(props_out);

	FREEP(buf);

	return UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

static UT_Bool s_doPrint(FV_View * pView, UT_Bool bTryToSuppressDialog)
{
	AP_Frame * pFrame = (AP_Frame *) pView->getParentData();
	UT_ASSERT(pFrame);

	pFrame->raise();

	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_Print * pDialog
		= (AP_Dialog_Print *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_PRINT));
	UT_ASSERT(pDialog);

	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

	pDialog->setDocumentTitle(pFrame->getTempNameFromTitle());
	pDialog->setDocumentPathname((doc->getFilename())
								 ? doc->getFilename()
								 : pFrame->getTempNameFromTitle());
	pDialog->setEnablePageRangeButton(UT_TRUE,1,pLayout->countPages());
	pDialog->setEnablePrintSelection(UT_FALSE);	// TODO change this when we know how to do it.
	pDialog->setEnablePrintToFile(UT_TRUE);
	pDialog->setTryToBypassActualDialog(bTryToSuppressDialog);

	pDialog->runModal(pFrame);

	AP_Dialog_Print::tAnswer ans = pDialog->getAnswer();
	UT_Bool bOK = (ans == AP_Dialog_Print::a_OK);

	if (bOK)
	{
		DG_Graphics * pGraphics = pDialog->getPrinterGraphicsContext();
		FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
		pDocLayout->formatAll();
		FV_View * pPrintView = new FV_View(pFrame,pDocLayout);
		UT_uint32 nFromPage, nToPage;
		(void)pDialog->getDoPrintRange(&nFromPage,&nToPage);

		// nFromPage and nToPage are range checked in the platform
		// dialog code, so we can freely assert this here.
		
		UT_ASSERT((UT_sint32) nToPage <= pDocLayout->countPages());
		
		// TODO add code to handle getDoPrintSelection()

		UT_uint32 nCopies = pDialog->getNrCopies();
		UT_Bool bCollate = pDialog->getCollate();

		dg_DrawArgs da;
		memset(&da, 0, sizeof(da));
		da.pG = NULL;
		da.width = pDocLayout->getWidth();
		da.height = pDocLayout->getHeight();

		// TODO these are here temporarily to make printing work.  We'll fix the hack later.
		// BUGBUG assumes all pages are same size and orientation
		da.width;
		da.height /= pLayout->countPages();

		UT_uint32 j,k;

		const char *pDocName = ((doc->getFilename()) ? doc->getFilename() : pFrame->getTempNameFromTitle());

		pGraphics->startPrint();
		if (bCollate)
		{
			for (j=1; (j <= nCopies); j++)
				for (k=nFromPage; (k <= nToPage); k++)
				{
					pGraphics->startPage(pDocName, k, UT_TRUE, da.width, da.height);
					pPrintView->draw(k-1, &da);
				}
		}
		else
		{
			for (k=nFromPage; (k <= nToPage); k++)
				for (j=1; (j <= nCopies); j++)
				{
					pGraphics->startPage(pDocName, k, UT_TRUE, da.width, da.height);
					pPrintView->draw(k-1, &da);
				}
		}
		pGraphics->endPrint();

		delete pDocLayout;
		delete pPrintView;

		pDialog->releasePrinterGraphicsContext(pGraphics);
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

/*****************************************************************/
/*****************************************************************/

Defun1(print)
{
	ABIWORD_VIEW;
	return s_doPrint(pView,UT_FALSE);
}

Defun1(printTB)
{
	// print (intended to be from the tool-bar (where we'd like to
	// suppress the dialog if possible))

	ABIWORD_VIEW;
	return s_doPrint(pView,UT_TRUE);
}

/****************************************************************/
/****************************************************************/

Defun1(toggleBold)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "font-weight", "bold", "normal");
}

Defun1(toggleItalic)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "font-style", "italic", "normal");
}

Defun1(toggleUline)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "underline", "none", UT_TRUE);
}

Defun1(toggleStrike)
{
	ABIWORD_VIEW;
	return _toggleSpan(pView, "text-decoration", "line-through", "none", UT_TRUE);
}

Defun0(togglePlain)
{
	// TODO: remove all character-level formatting
	// HYP: explicitly delete it, to get back to defaults, styles
	return UT_TRUE;
}

Defun1(alignLeft)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "left", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(alignCenter)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "center", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(alignRight)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "right", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(alignJustify)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "text-align", "justify", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(singleSpace)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "line-height", "1.0", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(middleSpace)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "line-height", "1.5", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(doubleSpace)
{
	ABIWORD_VIEW;
	const XML_Char * properties[] =	{ "line-height", "2.0", 0};
	pView->setBlockFormat(properties);
	return UT_TRUE;
}

Defun1(Test_Dump)
{
	ABIWORD_VIEW;
	pView->Test_Dump();
	return UT_TRUE;
}

