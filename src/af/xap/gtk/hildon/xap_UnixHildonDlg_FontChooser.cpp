/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2005 INdT
 * Author: Renato Araujo <renato.filho@indt.org.br>
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
#include <gtk/gtk.h>
#include <hildon/hildon-font-selection-dialog.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "xap_UnixDialogHelper.h"
#include "xap_UnixHildonDlg_FontChooser.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_EncodingManager.h"

//
// For Screen color picker
enum
{
		RED,
		GREEN,
		BLUE,
		OPACITY
};


/*****************************************************************/
XAP_Dialog * XAP_UnixHildonDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_UnixHildonDialog_FontChooser * p = new XAP_UnixHildonDialog_FontChooser(pFactory,id);
	return p;
}

XAP_UnixHildonDialog_FontChooser::XAP_UnixHildonDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_FontChooser(pDlgFactory,id)
{
}

XAP_UnixHildonDialog_FontChooser::~XAP_UnixHildonDialog_FontChooser(void)
{
}

		
void XAP_UnixHildonDialog_FontChooser::fillFontInfo()
{
	GdkColor color;
	UT_RGBColor c;
	gchar* pszFontName;
	PangoFontDescription *ptrFontDescNew;
	
	UT_parseColor (getVal("color").c_str(), c);
	color.red = static_cast<guint16> ((c.m_red / 255.0) * 65535.0);
	color.green = static_cast<guint16> ((c.m_grn / 255.0) * 65535.0);
	color.blue = static_cast<guint16> ((c.m_blu / 255.0) * 65535.0);

	
       	pszFontName = g_strdup_printf ("%s,%d", getVal("font-family").c_str(), 
						atoi(std_size_string(UT_convertToPoints(getVal("font-size").c_str()))));
						
	PangoFontDescription* ptrFontDesc = pango_font_description_from_string (pszFontName);

	PangoFont *fnt = pango_context_load_font (gtk_widget_get_pango_context (GTK_WIDGET(m_Widget)), 
						  ptrFontDesc);

	ptrFontDescNew = pango_font_describe  (fnt);
					     
	g_object_set (G_OBJECT (m_Widget), "family", pango_font_description_get_family (ptrFontDescNew),
					   "underline", m_bUnderline,
					   "strikethrough", m_bStrikeout,
					   "color", &color,
					   "size", atoi(std_size_string(UT_convertToPoints(getVal("font-size").c_str()))),
					   NULL);

	pango_font_description_free (ptrFontDesc);
	pango_font_description_free (ptrFontDescNew);
	g_free (pszFontName);
	
	//font style
	listStyle st = LIST_STYLE_NORMAL;
	if (getVal("font-style").empty() || getVal("font-weight").empty())
			st = LIST_STYLE_NONE;
	else if (!g_ascii_strcasecmp(getVal("font-style").c_str(), "normal") &&
					 !g_ascii_strcasecmp(getVal("font-weight").c_str(), "normal"))
			st = LIST_STYLE_NORMAL;
	else if (!g_ascii_strcasecmp(getVal("font-style").c_str(), "normal") &&
					 !g_ascii_strcasecmp(getVal("font-weight").c_str(), "bold"))
			st = LIST_STYLE_BOLD;
	else if (!g_ascii_strcasecmp(getVal("font-style").c_str(), "italic") &&
					 !g_ascii_strcasecmp(getVal("font-weight").c_str(), "normal"))
			st = LIST_STYLE_ITALIC;
	else if (!g_ascii_strcasecmp(getVal("font-style").c_str(), "italic") &&
					 !g_ascii_strcasecmp(getVal("font-weight").c_str(), "bold"))
			st = LIST_STYLE_BOLD_ITALIC;
	else
	{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	switch (st)
	{
	case LIST_STYLE_NORMAL:
		g_object_set (G_OBJECT (m_Widget), 
					"bold", FALSE,
					"italic", FALSE,
					NULL);
		break;
	case LIST_STYLE_BOLD:
		g_object_set (G_OBJECT (m_Widget), 
					"bold", TRUE,
					"italic", FALSE,
					NULL);
		break;
	case LIST_STYLE_ITALIC:
		g_object_set (G_OBJECT (m_Widget), 
					"bold", FALSE,
					"italic", TRUE,
					NULL);
		break;
	case LIST_STYLE_BOLD_ITALIC:
		g_object_set (G_OBJECT (m_Widget), 
					"bold", TRUE,
					"italic", TRUE,
					NULL);
		break;		
	default:
		break;
	}
}
		
/* Gets PangoFontDescription from iterator */
void XAP_UnixHildonDialog_FontChooser::loadFontInfo()
{
	const gchar* cszFontFamily;
	GdkColor *ptrFontColor;
	gboolean bFontBold;
	gboolean bFontItalic;
	gboolean bFontUnderline;
	gboolean bFontStrikethrough;
	gint iFontSize;
	
 	m_bStrikeout = m_bUnderline = false;
	m_bChangedUnderline = !m_bChangedUnderline;	
	m_bChangedStrikeOut = !m_bChangedStrikeOut;
	
	/* Search needed desc from list's attributes */
	g_object_get (G_OBJECT (m_Widget),
		      "color", &ptrFontColor,
		      "family", &cszFontFamily,
		      "bold", &bFontBold,
		      "italic", &bFontItalic,
		      "underline", &bFontUnderline,
		      "strikethrough", &bFontStrikethrough,
		      "size", &iFontSize,
		      NULL);
		      
	//font color
	gchar *buf_color = (gchar *) g_new(gchar, 8);
		
	m_currentFGColor[RED]   = static_cast<double> (ptrFontColor->red) / 65535.0;
	m_currentFGColor[GREEN] = static_cast<double> (ptrFontColor->green) / 65535.0;
	m_currentFGColor[BLUE]  = static_cast<double> (ptrFontColor->blue) /  65535.0;
		
	sprintf(buf_color, "%02x%02x%02x",
		static_cast<unsigned int>(m_currentFGColor[RED] * static_cast<gdouble>(255.0)),
		static_cast<unsigned int>(m_currentFGColor[GREEN] * static_cast<gdouble>(255.0)),
		static_cast<unsigned int>(m_currentFGColor[BLUE] * static_cast<gdouble>(255.0)));

	addOrReplaceVecProp("color",static_cast<gchar *>(buf_color));
	
	
	//font family
	char *szFontFamily = new char[strlen(cszFontFamily)  + 1 ];
	sprintf(szFontFamily, cszFontFamily);		
	addOrReplaceVecProp("font-family",static_cast<gchar *> (szFontFamily) );		
				
	//font size
	char *szFontSize = new char[50];
	memset(szFontSize, '\0', 50);
	g_snprintf(szFontSize, 50, "%dpt", iFontSize);		
	addOrReplaceVecProp("font-size", static_cast<gchar *> (szFontSize) );
				
	//font style
	if (bFontItalic)
		addOrReplaceVecProp("font-style","italic");
	else
		addOrReplaceVecProp("font-style","normal");

	if (bFontBold)
		addOrReplaceVecProp("font-weight","bold");
	else
		addOrReplaceVecProp("font-weight","normal");
			
			
	//bgcolor
	//TODO
	
	//font underline
	m_bUnderline = bFontUnderline;

	//font srtrikethrough
	m_bStrikeout = bFontStrikethrough;			
	
	setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);		
}



void XAP_UnixHildonDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	GtkWidget *pTopLevel = (static_cast<XAP_UnixFrameImpl*> (pFrame->getFrameImpl()))->getTopLevelWindow();
	//PangoAttrList *default_list = pango_attr_list_new();	

        m_Widget = GTK_WIDGET(hildon_font_selection_dialog_new(GTK_WINDOW(gtk_widget_get_parent(GTK_WIDGET(pTopLevel))),
		                                               NULL));
	
	gtk_widget_show_all(GTK_WIDGET(m_Widget));
	
	m_answer = a_CANCEL;
	
	m_doneFirstFont = true;
	
	fillFontInfo();
	
	if (gtk_dialog_run(GTK_DIALOG(m_Widget)) == GTK_RESPONSE_OK)
	{
		
		//default_list = hildon_font_selection_dialog_get_font ( HILDON_FONT_SELECTION_DIALOG ( m_Widget ) );		
		loadFontInfo();
		m_answer = a_OK;			
		
		//TODO
		//addOrReplaceVecProp("bgcolor",static_cast<gchar *>(buf_color));
	}	
	gtk_widget_destroy(GTK_WIDGET(m_Widget));
	m_doneFirstFont = false;
	
	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
							 getVal("font-family").c_str(),        ((m_bChangedFontFamily) ? "(chg)" : ""),
							 getVal("font-size").c_str(),            ((m_bChangedFontSize) ? "(chg)" : ""),
							 getVal("font-weight").c_str(),        ((m_bChangedFontWeight) ? "(chg)" : ""),
							 getVal("font-style").c_str(),          ((m_bChangedFontStyle) ? "(chg)" : ""),
							 getVal("color").c_str(),                           ((m_bChangedColor) ? "(chg)" : ""),
							 m_bUnderline,                                                        ((m_bChangedUnderline) ? "(chg)" : ""),
							 m_bStrikeout,                                                        ((m_bChangedStrikeOut) ? "(chg)" : "")));
	
}
