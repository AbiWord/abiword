#include <string.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../../af/xap/xp/xap_App.h"
#include "../../af/xap/xp/xap_Frame.h"
#include "../../text/fmt/xp/fv_View.h"
#include "../../af/util/xp/ut_string.h"

MODULE = abi		PACKAGE = abi::FV_View

bool
scroll(pView, where)
	FV_View *pView
	const char *where
	CODE:
		assert(where && where[0]);
		switch (where[0])
		{
		case 'd':
			if (strncmp("down", where, 4) == 0)
				pView->cmdScroll(AV_SCROLLCMD_LINEDOWN);
			break;
		case 'l':
			if (strncmp("left", where, 4) == 0)
				pView->cmdScroll(AV_SCROLLCMD_LINELEFT);
			break;
		case 'p':
			if (strncmp("pagedown", where, 8) == 0)
				pView->cmdScroll(AV_SCROLLCMD_PAGEDOWN);
			else if (strncmp("pageup", where, 6) == 0)
				pView->cmdScroll(AV_SCROLLCMD_PAGEUP);
			break;
		case 'r':
			if (strncmp("right", where, 5) == 0)
			{
				pView->cmdScroll(AV_SCROLLCMD_LINERIGHT);
				printf("scrolling right\n");
				fflush(stdout);
			}
			break;
		case 'u':
			if (strncmp("up", where, 2) == 0)
			{
				pView->cmdScroll(AV_SCROLLCMD_LINEUP);
				printf("scrolling up\n");
				fflush(stdout);
			}
			break;
		}

		RETVAL = true;
	OUTPUT:
		RETVAL

bool
setCharFormat (pView, ...)
	FV_View *pView
	CODE:
	{
		XML_Char **properties = new XML_Char* [items];

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
cmdSaveAs(pView, filename, left, cpy)
	FV_View *pView
	const char * filename
	int	left
	bool	cpy
	CODE:
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
		UT_UCS_cloneString_char(&text, pszText);
		pView->cmdCharInsert(text, strlen(pszText));
		free(text);
		RETVAL = true;
	OUTPUT:
		RETVAL

MODULE = abi		PACKAGE = abi::XAP_Frame

XAP_Frame *
openFile(pszFilename)
	const char *pszFilename
	CODE:
		XAP_App* app = XAP_App::getApp();
		RETVAL = app->newFrame();
		RETVAL->loadDocument(pszFilename, 0, true);
	OUTPUT:
		RETVAL

FV_View *
getCurrentView(pFrame)
	XAP_Frame *pFrame
	CODE:
		RETVAL = (FV_View *) pFrame->getCurrentView();
	OUTPUT:
		RETVAL


