#include <string.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* perl sux */
#undef ref
#undef list

#include "../../af/xap/xp/xap_App.h"
#include "../../af/xap/xp/xap_Frame.h"
#include "../../af/xap/xp/xad_Document.h"
#include "../../text/ptbl/xp/pd_Document.h"
#include "../../text/fmt/xp/fv_View.h"
#include "../../text/fmt/xp/fp_PageSize.h"
#include "../../af/util/xp/ut_string.h"
#include "../../af/util/xp/ut_units.h"
#include "../../af/util/xp/ut_PerlBindings.h"
#include "../../af/ev/xp/ev_EditMethod.h"

MODULE = AbiWord		PACKAGE = AbiWord::FV_View

void
moveCursorAbs(pView, target, where)
	FV_View *pView
	const char *target
	int where
	ALIAS:
		AbiWord::FV_View::moveCursorAbs = 0
		AbiWord::FV_View::moveCursorRel = 1
	CODE:
		UT_UCSChar *tmp;
		static char szWhere[16];
		const char * format = ix ? "%+d" : "%d";
		sprintf(szWhere, format, where);
		assert(target && target[0]);
		// printf("moveCursorAbs\n");

		switch (target[0])
		{
		case 'p': /* page */
			if (UT_UCS4_cloneString_char(&tmp, szWhere))
			{
				pView->gotoTarget(AP_JUMPTARGET_PAGE, tmp);
				g_free(tmp);
			}
			break;
		case 'l': /* line */
			if (UT_UCS4_cloneString_char(&tmp, szWhere))
			{
				pView->gotoTarget(AP_JUMPTARGET_LINE, tmp);
				g_free(tmp);
			}
			break;
		}

void
cut(pView)
	FV_View* pView
	CODE:
		pView->cmdCut();

void
copy(pView)
	FV_View* pView
	CODE:
		pView->cmdCopy();

void
paste(pView)
	FV_View* pView
	CODE:
		pView->cmdPaste();

void
setPaperColor(pView, color)
	FV_View* pView
	const char* color
	CODE:
		pView->setPaperColor((gchar*) color);

bool
setCharFormat (pView, ...)
	FV_View *pView
	ALIAS:
		AbiWord::FV_View::setCharFormat = 0
		AbiWord::FV_View::setSectionFormat = 1
		AbiWord::FV_View::setBlockFormat = 2
	CODE:
	{
		gchar **properties = new gchar* [items];
		// printf("setCharFormat\n");

		for (int i = 1; i < items; ++i)
			properties[i - 1] = SvPV(ST(i), PL_na);

		properties[items - 1] = NULL;

		switch (ix) {
		case 0:
			pView->setCharFormat((const gchar **) properties);
			break;
		case 1:
			pView->setSectionFormat((const gchar **) properties);
			break;
		case 2:
			pView->setBlockFormat((const gchar **) properties);
			break;
		}

		delete[] properties;
		RETVAL = true;
	}
	OUTPUT:
		RETVAL

void
changeNumColumns (pView, ncolumns)
	FV_View *pView
	unsigned int ncolumns
	CODE:
		/* this is not actually implemented, though it's in the header
		pView->changeNumColumns (ncolumns);*/

void
cmdCharDelete (pView, forward, count)
	FV_View *pView
	bool forward
	unsigned int	count
	CODE:
		pView->cmdCharDelete (forward, count);

unsigned int
getCurrentPageNumber (pView)
	FV_View *pView
	CODE:
		RETVAL = pView->getCurrentPageNumber();
	OUTPUT:
	RETVAL

bool
saveAs(pView, filename, left, cpy)
	FV_View *pView
	const char * filename
	int	left
	bool	cpy
	CODE:
		// printf("saveAs\n");
		pView->cmdSaveAs(filename, left, cpy);
		RETVAL = true;
	OUTPUT:
		RETVAL

bool
write(pView, pszText)
	FV_View *pView
	const char *pszText
	CODE:
		UT_UCSChar *text = NULL;
		// printf("write\n");
		UT_UCS4_cloneString_char(&text, pszText);
		pView->cmdCharInsert(text, strlen(pszText));
		g_free(text);
		RETVAL = true;
	OUTPUT:
		RETVAL

bool
write_OneAtTime(pView, pszText)
	FV_View *pView
	const char *pszText
	CODE:
		// THIS METHOD IS ONLY USEFUL FOR SPEED TESTS!!
		static UT_UCSChar text[2] = { 0, 0 };
		while ((text[0] = *pszText++) != '\0')
			pView->cmdCharInsert(text, 1);
		RETVAL = true;
	OUTPUT:
		RETVAL

void
editHeader(pView)
	FV_View *pView
	CODE:
		pView->cmdEditHeader();

void
editFooter(pView)
	FV_View *pView
	CODE:
		pView->cmdEditFooter();

void
editBody(pView)
	FV_View *pView
	CODE:
		pView->clearHdrFtrEdit();
		pView->warpInsPtToXY(0, 0, false);

unsigned int
getPoint(pView)
	FV_View *pView
	CODE:
		RETVAL = pView->getPoint();
	OUTPUT:
		RETVAL

bool
find(pView, pszText, matchCase)
	FV_View* pView
	const char* pszText
	bool matchCase
	CODE:
		UT_UCSChar *text = NULL;
		UT_UCS4_cloneString_char(&text, pszText);
		bool bTmp;
		pView->findSetMatchCase(matchCase);
		RETVAL = pView->findNext(text, bTmp);
		g_free(text);
	OUTPUT:
		RETVAL

bool
replace(pView, pszTextToFind, pszReplacement, matchCase)
	FV_View* pView
	const char* pszTextToFind
	const char* pszReplacement
	bool matchCase
	CODE:
		UT_UCSChar *textToFind = NULL;
		UT_UCS4_cloneString_char(&textToFind, pszTextToFind);
		UT_UCSChar *replacement = NULL;
		UT_UCS4_cloneString_char(&replacement, pszReplacement);
		bool bTmp;
		pView->findSetMatchCase(matchCase);
		pView->findSetFindString(textToFind);
		pView->findSetReplaceString(replacement);
		RETVAL = pView->findReplace(bTmp);
		g_free(textToFind);
		g_free(replacement);
	OUTPUT:
		RETVAL

char*
getSelectionText(pView)
	FV_View* pView
	CODE:
		if (!pView->isSelectionEmpty())
		{
			UT_UCSChar* text;
			pView->getSelectionText(text);
			UT_uint32 size = UT_UCS4_strlen(text);
			RETVAL = (char*) g_try_malloc(size);
			UT_UCS4_strcpy_to_char(RETVAL, text);
		}
		else
		{
			RETVAL = (char*) g_try_malloc(1);
			*RETVAL = '\0';
		}

	OUTPUT:
		RETVAL

void
print(pView)
	FV_View* pView
	ALIAS:
		AbiWord::FV_View::showPrintDialog = 0
		AbiWord::FV_View::print = 1
	CODE:
		EV_EditMethodContainer* pEMC = XAP_App::getApp()->getEditMethodContainer();
		EV_EditMethod* pEM = 0;

		if (ix == 0)
			pEM = pEMC->findEditMethodByName("print");
		else
			pEM = pEMC->findEditMethodByName("printTB");

		pEM->Fn(pView, 0);

MODULE = AbiWord		PACKAGE = AbiWord::XAP_Frame

XAP_Frame *
getLastFocussed()
	CODE:
		// printf("getLastFocussed\n");
		RETVAL = XAP_App::getApp()->getLastFocussedFrame();
	OUTPUT:
		RETVAL

XAP_Frame *
openFile(pszFilename)
	const char* pszFilename
	CODE:
		XAP_App* app = XAP_App::getApp();
		// printf("openFile\n");
		RETVAL = app->newFrame();
		RETVAL->loadDocument(pszFilename, 0, true);
	OUTPUT:
		RETVAL

FV_View *
getCurrentView(pFrame)
	XAP_Frame* pFrame
	CODE:
		// printf("getCurrentView\n");
		RETVAL = (FV_View *) pFrame->getCurrentView();
	OUTPUT:
		RETVAL

void
setPageSize(pFrame, iWidth, iHeight)
	XAP_Frame* pFrame
	double iWidth
	double iHeight
	CODE:
		// THIS METHOD DOESN'T WORK
		AD_Document* ad_doc = pFrame->getCurrentDoc();
		PD_Document* doc = dynamic_cast<PD_Document*> (ad_doc);
		if (doc)
		{
			fp_PageSize ps(iWidth, iHeight, DIM_MM);
//			doc->setPageSize(ps);
		}
		
void
setPageSizeByName(pFrame, pszName)
	XAP_Frame* pFrame
	const char* pszName
	CODE:
		// THIS METHOD DOESN'T WORKS
		AD_Document* ad_doc = pFrame->getCurrentDoc();
		PD_Document* doc = dynamic_cast<PD_Document*> (ad_doc);
		if (doc)
//			doc->setPageSize(fp_PageSize(pszName));
			;

void
close(pFrame)
	XAP_Frame *pFrame
	CODE:
		XAP_App * pApp = XAP_App::getApp();

		if (pFrame == pApp->getLastFocussedFrame())
			pApp->clearLastFocussedFrame();

		if (pApp->getFrameCount() <= 1)
		{
		  	// Delete all the open modeless dialogs
			pApp->closeModelessDlgs();
			pApp->reallyExit();
		}

		pApp->forgetFrame(pFrame);
		pFrame->close();
		delete pFrame;

void
register(pszFunctionName, pszMenuPath, pszDescription, bRaisesDialog)
	const char *pszFunctionName
	const char *pszMenuPath
	const char *pszDescription
	bool bRaisesDialog
	CODE:
		UT_PerlBindings::getInstance().registerCallback(
			pszFunctionName, pszMenuPath, pszDescription, bRaisesDialog);

void
exit()
	CODE:
		XAP_App::getApp()->reallyExit();

