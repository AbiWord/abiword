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
#include "xap_QNXFrame.h"

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

	PtWidget_t * parentWindow = m_pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);

	newfont = (char *)PtFontSelection(parentWindow,	/* Parent */
							  		  NULL, 		/* Position (centered) */
							  				/* Title */
 							  		  pSS->getValue(XAP_STRING_ID_DLG_UFS_FontTitle),
							  		  "helv10",		/* Initial font */
							  		  -1,			/* Symbol to select fonts by */							
							  		  PHFONT_ALL_FONTS, /* Which type of fonts */
							  		  NULL); 	/* Sample string */

	if (newfont) {
		FontQueryInfo finfo;
		char *s, *p, c;

		//NOTE: I could use PfQueryFont for all this information
		PfQueryFont(newfont, &finfo);

		//Split name[size][style] into pieces
		s = p = newfont;
		while (*p && (*p < '0' || *p > '9')) { p++; }
		c = *p; *p = '\0';
		s = finfo.desc;
		printf("Set family to %s \n", s);
		setFontFamily(s); m_bChangedFontFamily = true;

		s = p; *p = c;
		while (*p && (*p >= 0 && *p <= '9')) { p++; }
		c = *p; *p = '\0';
		//This is mental having to put the pt on the end
		char tempsize[20]; sprintf(tempsize, "%spt", s);
		setFontSize(tempsize); m_bChangedFontSize = true;

		setFontWeight("normal"); m_bChangedFontWeight = true;
		setFontStyle("normal"); m_bChangedFontStyle = true;
		while (*p) {
			switch (*p) {
			case 'b':
				setFontWeight("bold");
				break;
			case 'i':
				setFontStyle("italic");
				break;
			default:
				break;
			}
		}
		
		m_answer = XAP_Dialog_FontChooser::a_OK;
		//free(newfont);
	}
	else {
		m_answer = XAP_Dialog_FontChooser::a_CANCEL;
	}

	m_pQNXFrame = NULL;
}
	
#if 0
bool XAP_QNXDialog_FontChooser::getFont(XAP_QNXFont ** font)
{
	UT_ASSERT(font);
	
	gchar * fontText[2] = {NULL, NULL};
	XAP_QNXFont::style styleNumber;

	GList * selectedRow = NULL;
	int rowNumber = 0;

	selectedRow = GTK_CLIST(m_fontList)->selection;
	if (selectedRow)
	{
		rowNumber = GPOINTER_TO_INT(selectedRow->data);
		gtk_clist_get_text(GTK_CLIST(m_fontList), rowNumber, 0, fontText);
		UT_ASSERT(fontText && fontText[0]);
	}
	else
	{
		return false;
	}
		
	selectedRow = GTK_CLIST(m_styleList)->selection;
	if (selectedRow)
	{
		int style = GPOINTER_TO_INT(selectedRow->data);

		switch(style)
		{
		case LIST_STYLE_NORMAL:
			styleNumber = XAP_QNXFont::STYLE_NORMAL;
			break;
		case LIST_STYLE_BOLD:
			styleNumber = XAP_QNXFont::STYLE_BOLD;
			break;
		case LIST_STYLE_ITALIC:
			styleNumber = XAP_QNXFont::STYLE_ITALIC;
			break;
		case LIST_STYLE_BOLD_ITALIC:
			styleNumber = XAP_QNXFont::STYLE_BOLD_ITALIC;
			break;
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	}
	else
	{
		return false;
	}
	
	const XAP_QNXFont * tempQNXFont = m_fontManager->getFont((const char *) fontText[0], styleNumber);

	if (tempQNXFont)
	{
		// we got a font, set the variables and return success
		*font = (XAP_QNXFont *) tempQNXFont;
		return true;
	}

	return false;
}
#endif

bool XAP_QNXDialog_FontChooser::getDecoration(bool * strikeout,
												 bool * underline)
{
#if 0
	UT_ASSERT(strikeout && underline);

	*strikeout = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut));
	*underline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline));

#endif
	return true;
}

bool XAP_QNXDialog_FontChooser::getSize(UT_uint32 * pointsize)
{
#if 0
	UT_ASSERT(pointsize);

	GList * selectedRow = NULL;
	gchar * sizeText[2] = {NULL, NULL};
	int rowNumber = 0;
	
	selectedRow = GTK_CLIST(m_sizeList)->selection;
	if (selectedRow)
	{
		rowNumber = GPOINTER_TO_INT(selectedRow->data);
		gtk_clist_get_text(GTK_CLIST(m_sizeList), rowNumber, 0, sizeText);
		UT_ASSERT(sizeText && sizeText[0]);

		*pointsize = (UT_uint32) atol(sizeText[0]);
		return true;
	}

#endif
	return false;
}

bool XAP_QNXDialog_FontChooser::getEntryString(char ** string)
{
#if 0
	UT_ASSERT(string);

	// Maybe this will be editable in the future, if one wants to
	// hook up a mini formatter to the entry area.  Probably not.

	*string = PREVIEW_ENTRY_DEFAULT_STRING;

#endif
	return true;
}

bool XAP_QNXDialog_FontChooser::getForegroundColor(UT_RGBColor * color)
{
#if 0
	UT_ASSERT(color);
	
	gdouble currentColor[3] = { 0, 0, 0 };

	enum
	{
		RED = 0,
		GREEN,
		BLUE
	};
	
	gtk_color_selection_get_color(GTK_COLOR_SELECTION(m_colorSelector), currentColor);

	color->m_red = (unsigned char) (currentColor[RED]   * (gdouble) 255);
	color->m_grn = (unsigned char) (currentColor[GREEN] * (gdouble) 255);
	color->m_blu = (unsigned char) (currentColor[BLUE]  * (gdouble) 255);

	return true;
#endif
	return false;
}

bool XAP_QNXDialog_FontChooser::getBackgroundColor(UT_RGBColor * color)
{
#if 0
	// this just returns white now, it should later query the document
	// in the frame which launched this dialog
	
	UT_ASSERT(color);
	
	color->m_red = 255;
	color->m_grn = 255;
	color->m_blu = 255;

	return true;
#endif
	return false;
}


void XAP_QNXDialog_FontChooser::updatePreview(void)
{
#if 0
	// if we don't have anything yet, just ignore this request
	if (!m_gc)
		return;
	
//	bool strikeout = false;
//	bool underline = false;

	XAP_QNXFont * font = NULL;
	if (!getFont(&font))
		return;

	UT_uint32 pointsize = 0;
	if (!getSize(&pointsize))
		return;

	UT_uint32 pixelsize = (UT_uint32) ((double) pointsize / (double) 72 * (double) m_gc->getResolution());

	UT_ASSERT(pixelsize > 0);
	
	// Do some trickery to convert point sizes (as listed in the list box)
	// to real pixel sizes for this GC.  The layout engine does this
	// automatically because it's just that good.
	XAP_QNXFontHandle * entry = new XAP_QNXFontHandle(font, pixelsize);

	if (entry)
	{
		// set the new font
		m_gc->setFont(entry);

		// if we've set a font, this variable is true
		m_doneFirstFont = true;

		// now do the switch
		DELETEP(m_lastFont);
		m_lastFont = entry;
	}
	else
	{
		// we didn't get the font we requested, which is really weird
		// since this dialog can only let the user pick fonts the
		// font manager says it KNOWS it has.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
		

	// do the foreground (text) color
	UT_RGBColor fgcolor;
	if (!getForegroundColor(&fgcolor))
		return;
	m_gc->setColor(fgcolor);

	s_drawing_area_expose(m_preview, NULL);
	
#endif
}

