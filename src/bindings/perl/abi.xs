#include <string.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../../af/xap/xp/xap_App.h"
#include "../../af/xap/xp/xap_Frame.h"
#include "../../af/xap/xp/xad_Document.h"
#include "../../text/ptbl/xp/pd_Document.h"
#include "../../text/fmt/xp/fv_View.h"
#include "../../text/fmt/xp/fp_PageSize.h"
#include "../../af/util/xp/ut_string.h"

MODULE = abi		PACKAGE = abi::FV_View

void
moveCursorAbs(pView, target, where)
	FV_View *pView
	const char *target
	int where
	ALIAS:
		abi::FV_View::moveCursorAbs = 0
		abi::FV_View::moveCursorRel = 1
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
			if (UT_UCS_cloneString_char(&tmp, szWhere))
			{
				pView->gotoTarget(AP_JUMPTARGET_PAGE, tmp);
				free(tmp);
			}
			break;
		case 'l': /* line */
			if (UT_UCS_cloneString_char(&tmp, szWhere))
			{
				pView->gotoTarget(AP_JUMPTARGET_LINE, tmp);
				free(tmp);
			}
			break;
		}

bool
setCharFormat (pView, ...)
	FV_View *pView
	ALIAS:
		abi::FV_View::setCharFormat = 0
		abi::FV_View::setSectionFormat = 1
		abi::FV_View::setBlockFormat = 2
	CODE:
	{
		XML_Char **properties = new XML_Char* [items];
		// printf("setCharFormat\n");

		for (int i = 1; i < items; ++i)
			properties[i - 1] = SvPV(ST(i), PL_na);

		properties[items - 1] = NULL;

		switch (ix) {
		case 0:
			pView->setCharFormat((const XML_Char **) properties);
			break;
		case 1:
			pView->setSectionFormat((const XML_Char **) properties);
			break;
		case 2:
			pView->setBlockFormat((const XML_Char **) properties);
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
		UT_UCS_cloneString_char(&text, pszText);
		pView->cmdCharInsert(text, strlen(pszText));
		free(text);
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
		// THIS METHOD DOESN'T WORKS
		// pView->warpInsPtToXY(300, 300, true);
		// pView->moveInsPtTo(FV_DOCPOS_EOD);
		pView->clearHdrFtrEdit();

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
		UT_UCS_cloneString_char(&text, pszText);
		RETVAL = pView->findNext(text, matchCase);
		free(text);
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
		UT_UCS_cloneString_char(&textToFind, pszTextToFind);
		UT_UCSChar *replacement = NULL;
		UT_UCS_cloneString_char(&replacement, pszReplacement);
		RETVAL = pView->findReplace(textToFind, replacement, matchCase);
		free(textToFind);
		free(replacement);
	OUTPUT:
		RETVAL

char*
getSelectionText(pView)
	FV_View* pView
	CODE:
		UT_UCSChar* text = pView->getSelectionText();
		UT_uint32 size = UT_UCS_strlen(text);
		RETVAL = (char*) malloc(size);
		UT_UCS_strcpy_to_char(RETVAL, text);
	OUTPUT:
		RETVAL

MODULE = abi		PACKAGE = abi::XAP_Frame

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
	int iWidth
	int iHeight
	CODE:
		// THIS METHOD DOESN'T WORKS
		AD_Document* ad_doc = pFrame->getCurrentDoc();
		PD_Document* doc = dynamic_cast<PD_Document*> (ad_doc);
		if (doc)
		{
			fp_PageSize ps(iWidth, iHeight, fp_PageSize::mm);
			doc->setPageSize(ps);
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
			doc->setPageSize(fp_PageSize(pszName));

void
close(pFrame)
	XAP_Frame *pFrame
	CODE:
		XAP_App * pApp = pFrame->getApp();

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
register(pszFunctionName, pszDescription, pszHelp, pszAuthor, pszCopyright, pszDate, pszMenuPath, pFunction)
	const char *pszFunctionName
	const char *pszDescription
	const char *pszHelp
	const char *pszAuthor
	const char *pszCopyright
	const char *pszDate
	const char *pszMenuPath
	SV *pFunction
	CODE:
		// XAP_PerlBindings::get_instance().register_function(pszFunctionName, pFunction);

void
exit()
	CODE:
		XAP_App::getApp()->reallyExit();

