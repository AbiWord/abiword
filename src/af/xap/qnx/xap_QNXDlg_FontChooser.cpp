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

	XAP_QNXApp * pApp = (XAP_QNXApp *)m_pApp;
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

		printf("Selected font [%s] \n", newfont);
		//Split name[size][style] into pieces
		s = p = newfont;
		while (*p && (*p < '0' || *p > '9')) { p++; }
		c = *p; *p = '\0';
		s = finfo.desc;
		printf("Set family to %s \n", s);
		setFontFamily(s); m_bChangedFontFamily = UT_TRUE;

		s = p; *p = c;
		while (*p && (*p >= 0 && *p <= '9')) { p++; }
		c = *p; *p = '\0';
		//This is mental having to put the pt on the end
		char tempsize[20]; sprintf(tempsize, "%spt", s);
		printf("Set size to %spt \n", tempsize);
		setFontSize(tempsize); m_bChangedFontSize = UT_TRUE;

		setFontWeight("normal"); m_bChangedFontWeight = UT_TRUE;
		setFontStyle("normal"); m_bChangedFontStyle = UT_TRUE;
		while (*p) {
			switch (*p) {
			case 'b':
				printf("Set weight to %s \n", s);
				setFontWeight("bold");
				break;
			case 'i':
				printf("Set style to %s \n", s);
				setFontStyle("italic");
				break;
			default:
				break;
			}
		}
		
		printf("Finished \n");
		m_answer = XAP_Dialog_FontChooser::a_OK;
		//free(newfont);
	}
	else {
		printf("Didn't select any font \n");
		m_answer = XAP_Dialog_FontChooser::a_CANCEL;
	}
	return;

#if 0
	UT_DEBUGMSG(("FontChooserStart: Family[%s] Size[%s] Weight[%s] Style[%s] Color[%s] Underline[%d] StrikeOut[%d]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),
				 ((m_pColor) ? m_pColor : "" ),
				 (m_bUnderline),
				 (m_bStrikeOut)));

	// These define the color element offsets in a vector
	guint RED = 0;
	guint GREEN = 1;
	guint BLUE = 2;
	gdouble currentColor[3] = { 0, 0, 0 };
	gdouble funkyColor[3] = { -1, -1, -1 };

	// this is used many times below to grab pointers to
	// strings inside list elements
	gchar * text[2] = {NULL, NULL};
	// used similarly to convert between text and numeric arguments
	char sizeString[SIZE_STRING_SIZE];
	
	// Set up our own color space so we work well on 8-bit
	// displays.
    gtk_widget_push_visual(gtk_preview_get_visual());
    gtk_widget_push_colormap(gtk_preview_get_cmap());

	// establish the font manager before dialog creation
	XAP_App * app = m_pQNXFrame->getApp();
	XAP_QNXApp * unixapp = static_cast<XAP_QNXApp *> (app);
	m_fontManager = unixapp->getFontManager();

	// build the dialog
	GtkWidget * cf = create_windowFontSelection();
	UT_ASSERT(cf);

	// freeze updates of the preview
	m_blockUpdate = UT_TRUE;
	
	gtk_clist_clear(GTK_CLIST(m_fontList));

	// to sort out dupes
	UT_HashTable fontHash(256);

	// throw them in the hash save duplicates
	XAP_QNXFont ** fonts = m_fontManager->getAllFonts();
	for (UT_uint32 i = 0; i < m_fontManager->getCount(); i++)
	{
		if (!fontHash.findEntry(fonts[i]->getName()))
			fontHash.addEntry((char *) fonts[i]->getName(),
							  (char *) fonts[i]->getName(), NULL);
	}
	DELETEP(fonts);

	// fetch them out
	UT_HashEntry * entry;
	for (UT_uint32 k = 0; k < (UT_uint32) fontHash.getEntryCount(); k++)
	{
		entry = fontHash.getNthEntry((int) k);
		UT_ASSERT(entry);
		text[0] = (gchar *) entry->pszLeft;
		gtk_clist_append(GTK_CLIST(m_fontList), text);
	}

	// Set the defaults in the list boxes according to dialog data
	int foundAt = 0;

	// is this safe with an XML_Char * string?
	foundAt = searchCList(GTK_CLIST(m_fontList), (char *) m_pFontFamily);

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_fontList), foundAt, 0);
		gtk_clist_moveto(GTK_CLIST(m_fontList), foundAt, 0, 0, -1);
	}
	
	// this is pretty messy
	listStyle st = LIST_STYLE_NORMAL;
	if (!m_pFontStyle || !m_pFontWeight)
	{
		// select nothing
	}
	else if (!UT_stricmp(m_pFontStyle, "normal") &&
			 !UT_stricmp(m_pFontWeight, "normal"))
	{
		st = LIST_STYLE_NORMAL;
	}
	else if (!UT_stricmp(m_pFontStyle, "normal") &&
			 !UT_stricmp(m_pFontWeight, "bold"))
	{
		st = LIST_STYLE_BOLD;
	}
	else if (!UT_stricmp(m_pFontStyle, "italic") &&
			 !UT_stricmp(m_pFontWeight, "normal"))
	{
		st = LIST_STYLE_ITALIC;		
	}
	else if (!UT_stricmp(m_pFontStyle, "italic") &&
			 !UT_stricmp(m_pFontWeight, "bold"))
	{
		st = LIST_STYLE_BOLD_ITALIC;		
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	gtk_clist_select_row(GTK_CLIST(m_styleList), st, 0);
	gtk_clist_moveto(GTK_CLIST(m_styleList), st, 0, 0, -1);
	
	double size = UT_convertToPoints(m_pFontSize);
	g_snprintf(sizeString, SIZE_STRING_SIZE, "%ld", (long) size);
	foundAt = searchCList(GTK_CLIST(m_sizeList), sizeString);

	if (foundAt >= 0)
	{
		gtk_clist_select_row(GTK_CLIST(m_sizeList), foundAt, 0);
		gtk_clist_moveto(GTK_CLIST(m_sizeList), foundAt, 0, 0, -1);
	}
	
	// Set color in the color selector
	if (m_pColor)
	{
		UT_RGBColor c;
		UT_parseColor(m_pColor, c);

		currentColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
		currentColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
		currentColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_colorSelector), currentColor);
	}
	else
	{
		// if we have no color, use a placeholder of funky values
		// the user can't pick interactively.  This catches ALL
		// the cases except where the user specifically enters -1 for
		// all Red, Green and Blue attributes manually.  This user
		// should expect it not to touch the color.  :)
		gtk_color_selection_set_color(GTK_COLOR_SELECTION(m_colorSelector), funkyColor);
	}

	// set the strikeout and underline check buttons
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut), m_bStrikeOut);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_checkUnderline), m_bUnderline);	
	
	// get top level window and its GtkWidget *
	XAP_QNXFrame * frame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);
	// center it
    centerDialog(parent, GTK_WIDGET(cf));
	gtk_window_set_transient_for(GTK_WINDOW(cf), GTK_WINDOW(parent));
	
	// Run the dialog
	gtk_widget_show(GTK_WIDGET(cf));
	gtk_grab_add(GTK_WIDGET(cf));

	m_doneFirstFont = UT_FALSE;
	
	// attach a new graphics context
	m_gc = new GR_QNXGraphics(m_preview->window, m_fontManager);
	gtk_object_set_user_data(GTK_OBJECT(m_preview), this);
	
	// unfreeze updates of the preview
	m_blockUpdate = UT_FALSE;
	// manually trigger an update
	updatePreview();
	
	gtk_main();

	if (m_answer == XAP_Dialog_FontChooser::a_OK)
	{
		GList * selectedRow = NULL;
		int rowNumber = 0;
		
		selectedRow = GTK_CLIST(m_fontList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_fontList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);
			if (!m_pFontFamily || UT_stricmp(m_pFontFamily, text[0]))
			{
				setFontFamily(text[0]);
				m_bChangedFontFamily = UT_TRUE;
			}
		}
		
		selectedRow = GTK_CLIST(m_styleList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_styleList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);

		// perhaps these attributes really should be smashed
		// into bitfields.  :)
			if (rowNumber == LIST_STYLE_NORMAL)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "normal"))
				{
					setFontStyle("normal");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "normal"))
				{
					setFontWeight("normal");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else if (rowNumber == LIST_STYLE_BOLD)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "normal"))
				{
					setFontStyle("normal");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "bold"))
				{
					setFontWeight("bold");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else if (rowNumber == LIST_STYLE_ITALIC)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "italic"))
				{
					setFontStyle("italic");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "normal"))
				{
					setFontWeight("normal");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else if (rowNumber == LIST_STYLE_BOLD_ITALIC)
			{
				if (!m_pFontStyle || UT_stricmp(m_pFontStyle, "italic"))
				{
					setFontStyle("italic");
					m_bChangedFontStyle = UT_TRUE;
				}
				if (!m_pFontWeight || UT_stricmp(m_pFontWeight, "bold"))
				{
					setFontWeight("bold");
					m_bChangedFontWeight = UT_TRUE;
				}
			}
			else
			{
				UT_ASSERT(0);
			}
		}
		
		selectedRow = GTK_CLIST(m_sizeList)->selection;
		if (selectedRow)
		{
			rowNumber = GPOINTER_TO_INT(selectedRow->data);
			gtk_clist_get_text(GTK_CLIST(m_sizeList), rowNumber, 0, text);
			UT_ASSERT(text && text[0]);

			g_snprintf(sizeString, SIZE_STRING_SIZE, "%spt", text[0]);

			if (!m_pFontSize || UT_stricmp(m_pFontSize, sizeString))
			{
				setFontSize(sizeString);
				m_bChangedFontSize = UT_TRUE;
			}
		}
		
		gtk_color_selection_get_color(GTK_COLOR_SELECTION(m_colorSelector), currentColor);

		// test for funkyColor-has-been-changed-to-sane-color case
		if (currentColor[RED] >= 0 &&
			currentColor[GREEN] >= 0 &&
			currentColor[BLUE] >= 0)
		{
			char buf_color[6];
			sprintf(buf_color, "%02x%02x%02x",
					(unsigned int) (currentColor[RED] 	* (gdouble) 255.0),
					(unsigned int) (currentColor[GREEN]	* (gdouble) 255.0),
					(unsigned int) (currentColor[BLUE] 	* (gdouble) 255.0));
		
			if (!m_pColor || UT_stricmp(m_pColor, buf_color))
			{
				setColor(buf_color);
				m_bChangedColor = UT_TRUE;
			}
		}

		m_bChangedStrikeOut = (m_bStrikeOut != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut)));
		m_bChangedUnderline = (m_bUnderline != gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline)));
		if (m_bChangedStrikeOut)
			m_bStrikeOut = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut));
		if (m_bChangedUnderline)
			m_bUnderline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline));
	}

	gtk_widget_destroy (GTK_WIDGET(cf));

	// these dialogs are cached around through the dialog framework,
	// and this variable needs to get set back
	m_doneFirstFont = UT_FALSE;
	
    gtk_widget_pop_visual();
    gtk_widget_pop_colormap();
	
	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((m_pColor) ? m_pColor : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeOut),							((m_bChangedStrikeOut) ? "(chg)" : "")));

	// answer should be set by the appropriate callback
	// the caller can get the answer from getAnswer().

	m_pQNXFrame = NULL;
#endif
}
	
#if 0
UT_Bool XAP_QNXDialog_FontChooser::getFont(XAP_QNXFont ** font)
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
		return UT_FALSE;
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
			return UT_FALSE;
		}
	}
	else
	{
		return UT_FALSE;
	}
	
	const XAP_QNXFont * tempQNXFont = m_fontManager->getFont((const char *) fontText[0], styleNumber);

	if (tempQNXFont)
	{
		// we got a font, set the variables and return success
		*font = (XAP_QNXFont *) tempQNXFont;
		return UT_TRUE;
	}

	return UT_FALSE;
}
#endif

UT_Bool XAP_QNXDialog_FontChooser::getDecoration(UT_Bool * strikeout,
												 UT_Bool * underline)
{
#if 0
	UT_ASSERT(strikeout && underline);

	*strikeout = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkStrikeOut));
	*underline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_checkUnderline));

#endif
	return UT_TRUE;
}

UT_Bool XAP_QNXDialog_FontChooser::getSize(UT_uint32 * pointsize)
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
		return UT_TRUE;
	}

#endif
	return UT_FALSE;
}

UT_Bool XAP_QNXDialog_FontChooser::getEntryString(char ** string)
{
#if 0
	UT_ASSERT(string);

	// Maybe this will be editable in the future, if one wants to
	// hook up a mini formatter to the entry area.  Probably not.

	*string = PREVIEW_ENTRY_DEFAULT_STRING;

#endif
	return UT_TRUE;
}

UT_Bool XAP_QNXDialog_FontChooser::getForegroundColor(UT_RGBColor * color)
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

	return UT_TRUE;
#endif
	return UT_FALSE;
}

UT_Bool XAP_QNXDialog_FontChooser::getBackgroundColor(UT_RGBColor * color)
{
#if 0
	// this just returns white now, it should later query the document
	// in the frame which launched this dialog
	
	UT_ASSERT(color);
	
	color->m_red = 255;
	color->m_grn = 255;
	color->m_blu = 255;

	return UT_TRUE;
#endif
	return UT_FALSE;
}


void XAP_QNXDialog_FontChooser::updatePreview(void)
{
#if 0
	// if we don't have anything yet, just ignore this request
	if (!m_gc)
		return;
	
//	UT_Bool strikeout = UT_FALSE;
//	UT_Bool underline = UT_FALSE;

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
		m_doneFirstFont = UT_TRUE;

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

