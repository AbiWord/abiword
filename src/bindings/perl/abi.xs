#include <string.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../../af/xap/xp/xap_App.h"
#include "../../af/xap/xp/xap_Frame.h"
#include "../../text/fmt/xp/fv_View.h"
#include "../../af/util/xp/ut_string.h"

MODULE = abi		PACKAGE = abi::FV_View

void
moveCursorAbs(pView, target, where)
	FV_View *pView
	const char *target
	int where
	CODE:
		UT_UCSChar *tmp;
		char szWhere[16];
		sprintf(szWhere, "%d", where);
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

void
moveCursorRel(pView, target, where)
	FV_View *pView
	const char *target
	int where
	CODE:
		UT_UCSChar *tmp;
		char szWhere[16];
		sprintf(szWhere, "%+d", where);
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
	CODE:
	{
		XML_Char **properties = new XML_Char* [items];
		// printf("setCharFormat\n");

		for (int i = 1; i < items; ++i)
			properties[i - 1] = SvPV(ST(i), PL_na);

		properties[items - 1] = NULL;
		pView->setCharFormat((const XML_Char **) properties);

		delete[] properties;
		RETVAL = true;
	}
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

unsigned int
getPoint(pView)
	FV_View *pView
	CODE:
		RETVAL = pView->getPoint();
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
	const char *pszFilename
	CODE:
		XAP_App* app = XAP_App::getApp();
		// printf("openFile\n");
		RETVAL = app->newFrame();
		RETVAL->loadDocument(pszFilename, 0, true);
	OUTPUT:
		RETVAL

FV_View *
getCurrentView(pFrame)
	XAP_Frame *pFrame
	CODE:
		// printf("getCurrentView\n");
		RETVAL = (FV_View *) pFrame->getCurrentView();
	OUTPUT:
		RETVAL


