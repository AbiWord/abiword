/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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
#include "ut_bytebuf.h"
#include "ev_EditMethod.h"
#include "xav_View.h"
#include "gr_Graphics.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_Menu_Layouts.h"
#include "xap_Prefs.h"

#include "ap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_About.h"

#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Dlg_FontChooser.h"
#include "xap_Dlg_Print.h"
#include "xap_Dlg_WindowMore.h"

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
	static EV_EditMethod_Fn fileNew;
	static EV_EditMethod_Fn fileOpen;
	static EV_EditMethod_Fn fileSave;
	static EV_EditMethod_Fn fileSaveAs;
	static EV_EditMethod_Fn fileInsertImage;
    static EV_EditMethod_Fn insertData;

	static EV_EditMethod_Fn dlgOptions;
	static EV_EditMethod_Fn dlgAbout;

	static EV_EditMethod_Fn closeWindow;

	static EV_EditMethod_Fn noop;

	// Test routines

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	static EV_EditMethod_Fn Test_Dump;
#endif
};

/*****************************************************************/
/*****************************************************************/

#define _D_				EV_EMT_REQUIREDATA

#define F(fn)			ap_EditMethods::fn
#define N(fn)			#fn
#define NF(fn)			N(fn), F(fn)

static EV_EditMethod s_arrayEditMethods[] =
{
	EV_EditMethod(NF(dlgAbout),				0,		""),

	EV_EditMethod(NF(closeWindow),			0,	""),

	EV_EditMethod(NF(noop),					0,	""),

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	EV_EditMethod(NF(Test_Dump),			0,	"")
#endif
};

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))


EV_EditMethodContainer * AP_GetEditMethods(void)
{
	// Construct a container for all of the methods this application
	// knows about.

	return new EV_EditMethodContainer(NrElements(s_arrayEditMethods),s_arrayEditMethods);
}

#undef _D_
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



#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
Defun1(Test_Dump)
{
	return UT_TRUE;
}
#endif

Defun1(noop)
{
	// this is a no-op, so unbound menus don't assert at trade shows
	return UT_TRUE;
}

static UT_Bool s_doAboutDlg(XAP_Frame* pFrame, XAP_Dialog_Id id)
{
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_About * pDialog
		= (XAP_Dialog_About *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	// run the dialog (it should really be modeless if anyone
	// gets the urge to make it safe that way)
	pDialog->runModal(pFrame);

	UT_Bool bOK = UT_TRUE;

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

Defun1(dlgAbout)
{
	XAP_Frame * pFrame = (XAP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);

	s_doAboutDlg(pFrame, XAP_DIALOG_ID_ABOUT);

	return UT_TRUE;
}

Defun(closeWindow)
{
	XAP_Frame * pFrame = (XAP_Frame *) pAV_View->getParentData();
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
	UT_ASSERT(pApp);

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


