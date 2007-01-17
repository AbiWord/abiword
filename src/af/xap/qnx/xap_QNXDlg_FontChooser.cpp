/* AbiSource Application Framework
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
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "xap_QNXDlg_FontChooser.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "gr_QNXGraphics.h"

#define SIZE_STRING_SIZE	10

#define PREVIEW_BOX_BORDER_WIDTH_PIXELS 8
#define PREVIEW_BOX_HEIGHT_PIXELS	80

// your typographer's standard nonsense latin font phrase
#define PREVIEW_ENTRY_DEFAULT_STRING	"Lorem ipsum dolor sit amet, consectetaur adipisicing..."

/*****************************************************************/
XAP_Dialog * XAP_QNXDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_QNXDialog_FontChooser * p = new XAP_QNXDialog_FontChooser(pFactory,id);
	return p;
}

XAP_QNXDialog_FontChooser::XAP_QNXDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_FontChooser(pDlgFactory,id)
{
	m_gc = NULL;
}

XAP_QNXDialog_FontChooser::~XAP_QNXDialog_FontChooser(void)
{
	DELETEP(m_gc);
}


void XAP_QNXDialog_FontChooser::runModal(XAP_Frame * pFrame)
{

	/* This is a really simplified version of the font selector using
	   the native font selection widget.  Later we will have to do
       something a little more bold and creative with our own dialog */
	m_pQNXFrame = (XAP_QNXFrame *)pFrame;
	UT_ASSERT(m_pQNXFrame);

	XAP_QNXApp * pApp;
	pApp = (XAP_QNXApp *)m_pApp;
	UT_ASSERT(pApp);

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	char *newfont;
	UT_UTF8String s;

	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	UT_uint32	flags=0;
	UT_uint32 size;
	char fontname[MAX_FONT_TAG];

	if(!g_ascii_strcasecmp(getVal("font-weight"),"bold"))
		flags |= PF_STYLE_BOLD;
	if(!g_ascii_strcasecmp(getVal("font-style"),"italic"))
		flags |= PF_STYLE_ITALIC;	
	size = (UT_uint32)UT_convertToPoints(getVal("font-size"));
		

	PfGenerateFontName(getVal("font-family"),flags,size,fontname);

	pSS->getValueUTF8(XAP_STRING_ID_DLG_UFS_FontTitle,s);
	newfont = (char *)PtFontSelection(parentWindow,	/* Parent */
							  		  NULL, 		/* Position (centered) */
							  				/* Title */
									  s.utf8_str(),
							  		  fontname,		/* Initial font */
							  		  PHFONT_ALL_SYMBOLS,			/* Symbol to select fonts by */							
							  		  PHFONT_SCALABLE, /* Which type of fonts */
							  		  PREVIEW_ENTRY_DEFAULT_STRING); 	/* Sample string */

	if (newfont) {
		FontID *fontid=PfDecomposeStemToID(newfont);

		setFontFamily(PfFontDescription(fontid)); m_bChangedFontFamily = true;

		char tempsize[20]; sprintf(tempsize, "%dpt", PfFontSize(fontid));
		setFontSize(tempsize); m_bChangedFontSize = true;

		setFontWeight("normal"); m_bChangedFontWeight = true;
		setFontStyle("normal"); m_bChangedFontStyle = true;

		switch (PfFontFlags(fontid)) {
			case PF_STYLE_BOLD|PF_STYLE_ITALIC:
				setFontWeight("bold");
				setFontStyle("italic");
				break;
			case PF_STYLE_BOLD:
				setFontWeight("bold");
				break;
			case PF_STYLE_ITALIC:
				setFontStyle("italic");
				break;
			default:
				break;
		}
		
		m_answer = XAP_Dialog_FontChooser::a_OK;
		PfFreeFont(fontid);
		g_free(newfont);
	}
	else {
		m_answer = XAP_Dialog_FontChooser::a_CANCEL;
	}
}
	
bool XAP_QNXDialog_FontChooser::getDecoration(bool * strikeout,
												 bool * underline)
{
	return true;
}

bool XAP_QNXDialog_FontChooser::getSize(UT_uint32 * pointsize)
{
	return false;
}

bool XAP_QNXDialog_FontChooser::getEntryString(char ** string)
{
	return true;
}

bool XAP_QNXDialog_FontChooser::getForegroundColor(UT_RGBColor * color)
{
	return false;
}

bool XAP_QNXDialog_FontChooser::getBackgroundColor(UT_RGBColor * color)
{
	return false;
}


void XAP_QNXDialog_FontChooser::updatePreview(void)
{
}

