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
#include <hildon-widgets/hildon-font-selection-dialog.h>

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
#include "gr_UnixGraphics.h"

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

		
void XAP_UnixHildonDialog_FontChooser::fillFontInfo(PangoAttrList* list)
{
	PangoFontDescription *font = pango_font_description_new();
	
	//font family
    pango_font_description_set_family(font, getVal("font-family"));

	//font underline
	if (m_bUnderline) {
		PangoAttribute *pattrUnderline = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
		pattrUnderline->start_index = 0;
    	pattrUnderline->end_index = 1;
		pango_attr_list_insert(list, pattrUnderline);
	}		
	
	//font strikeout
	if (m_bStrikeout) {
		PangoAttribute *pattrStrike = pango_attr_strikethrough_new(PANGO_UNDERLINE_SINGLE);
		pattrStrike->start_index = 0;
    	pattrStrike->end_index = 1;
		pango_attr_list_insert(list,pattrStrike);		
	}
	
	//font color
	UT_RGBColor c;
    UT_parseColor(getVal("color"), c);

	gdouble color[3];
	color[RED] = static_cast<double> (c.m_red) / 255.0;
	color[GREEN] = static_cast<double> (c.m_grn) / 255.0;
	color[BLUE] = static_cast<double> (c.m_blu) / 255.0;	
	PangoAttribute *pattrColor = pango_attr_foreground_new(static_cast<unsigned int> (color[RED] * 65535.0), 
														   static_cast<unsigned int> (color[GREEN] * 65535.0), 
														   static_cast<unsigned int> (color[BLUE] * 65535.0));
	
	pattrColor->start_index = 0;
    pattrColor->end_index = 1;	
	pango_attr_list_insert(list, pattrColor);
	
	
	//font size
	char sizeString[50];	
	g_snprintf(sizeString, 50, "%s", std_size_string(UT_convertToPoints(getVal("font-size"))));
	pango_font_description_set_size(font, atoi(std_size_string(UT_convertToPoints(getVal("font-size")))) * PANGO_SCALE);
	
	//font style
	listStyle st = LIST_STYLE_NORMAL;
	if (!getVal("font-style") || !getVal("font-weight"))
			st = LIST_STYLE_NONE;
	else if (!UT_stricmp(getVal("font-style"), "normal") &&
					 !UT_stricmp(getVal("font-weight"), "normal"))
			st = LIST_STYLE_NORMAL;
	else if (!UT_stricmp(getVal("font-style"), "normal") &&
					 !UT_stricmp(getVal("font-weight"), "bold"))
			st = LIST_STYLE_BOLD;
	else if (!UT_stricmp(getVal("font-style"), "italic") &&
					 !UT_stricmp(getVal("font-weight"), "normal"))
			st = LIST_STYLE_ITALIC;
	else if (!UT_stricmp(getVal("font-style"), "italic") &&
					 !UT_stricmp(getVal("font-weight"), "bold"))
			st = LIST_STYLE_BOLD_ITALIC;
	else
	{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	switch (st)
	{
	case LIST_STYLE_NORMAL:
		pango_font_description_set_style(font, PANGO_STYLE_NORMAL);
		break;
	case LIST_STYLE_BOLD:
		pango_font_description_set_style(font, PANGO_STYLE_NORMAL);
		pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
		break;
	case LIST_STYLE_ITALIC:
		pango_font_description_set_style(font, PANGO_STYLE_ITALIC);
		break;
	case LIST_STYLE_BOLD_ITALIC:
		pango_font_description_set_style(font, PANGO_STYLE_ITALIC);
		pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
		break;		
	default:
		break;
	}
	
	PangoAttribute *pattrFont = pango_attr_font_desc_new(font);
	pattrFont->start_index = 0;
    	pattrFont->end_index = 1;
	pango_attr_list_insert(list, pattrFont);			
}
		
/* Gets PangoFontDescription from iterator */
void XAP_UnixHildonDialog_FontChooser::loadFontInfo(PangoAttrList* list)
{
    PangoFontDescription *font = NULL;
    PangoAttribute *pattr = NULL;
    PangoAttrIterator *iter = NULL;
    GSList *attrs = NULL;
    GSList *curattr = NULL;

    font = pango_font_description_new();
    if (font == NULL) {
        return;
    }
    iter = pango_attr_list_get_iterator( list );
    if ( iter == NULL ) {
        return;
    }
    attrs = pango_attr_iterator_get_attrs( iter );
    if ( attrs == NULL ) {
        return;
    }
	
	m_bStrikeout = m_bUnderline = false;
	m_bChangedUnderline = !m_bChangedUnderline;	
	m_bChangedStrikeOut = !m_bChangedStrikeOut;
	
    /* Search needed desc from list's attributes */
    for (curattr = attrs; curattr != NULL; curattr = curattr->next) {
        pattr = (PangoAttribute *) curattr->data;

        switch (pattr->klass->type) {
		case PANGO_ATTR_FOREGROUND:
			{
			gchar *buf_color = (gchar *) g_new(gchar, 8);
			PangoAttrColor *pattrColor = (PangoAttrColor *) pattr;
				
			m_currentFGColor[RED]   = static_cast<double> (pattrColor->color.red) / 65535.0;
			m_currentFGColor[GREEN] = static_cast<double> (pattrColor->color.green) / 65535.0;
			m_currentFGColor[BLUE]  = static_cast<double> (pattrColor->color.blue) /  65535.0;
				
			sprintf(buf_color, "%02x%02x%02x",
                                static_cast<unsigned int>(m_currentFGColor[RED] * static_cast<gdouble>(255.0)),
                                static_cast<unsigned int>(m_currentFGColor[GREEN] * static_cast<gdouble>(255.0)),
                                static_cast<unsigned int>(m_currentFGColor[BLUE] * static_cast<gdouble>(255.0)));

			addOrReplaceVecProp("color",static_cast<XML_Char *>(buf_color));
			break;			
			}
        case PANGO_ATTR_FONT_DESC:
			{
            pango_attr_iterator_get_font(iter, font, NULL, NULL);
		
			//font family
			const char *cszFontFamily = pango_font_description_get_family(font);		
			char *szFontFamily = new char[strlen(cszFontFamily)  + 1 ];
			sprintf(szFontFamily, cszFontFamily);		
			addOrReplaceVecProp("font-family",static_cast<XML_Char *> (szFontFamily) );		
				
			//font size
			gint iFontSize = pango_font_description_get_size(font) / PANGO_SCALE;
			char *szFontSize = new char[50];
			memset(szFontSize, '\0', 50);
			g_snprintf(szFontSize, 50, "%dpt", iFontSize);		
			addOrReplaceVecProp("font-size", static_cast<XML_Char *> (szFontSize) );
				
			//font style
			PangoStyle fontStyle = pango_font_description_get_style(font);
			PangoWeight fontWeight = pango_font_description_get_weight(font);

			if (fontStyle == PANGO_STYLE_NORMAL)
			{
				addOrReplaceVecProp("font-style","normal");
				
			}
			else if (fontStyle == PANGO_STYLE_OBLIQUE)
			{
				addOrReplaceVecProp("font-style","normal");
			}
			else if (fontStyle == PANGO_STYLE_ITALIC)
			{
				addOrReplaceVecProp("font-style","italic");
			}
			else
					UT_ASSERT(0);
			
			if (fontWeight == PANGO_WEIGHT_BOLD)
			{
				addOrReplaceVecProp("font-weight","bold");
			}
			else
			{
				addOrReplaceVecProp("font-weight","normal");
			}
			
			
			//bgcolor
			addOrReplaceVecProp("bgcolor","transparent");
          	break;
			}
		case PANGO_ATTR_UNDERLINE:
			m_bUnderline = true;
			break;		
		case PANGO_ATTR_STRIKETHROUGH:		
			m_bStrikeout  = true;			
			break;		
        default:
            break;
        }
		setFontDecoration(m_bUnderline,m_bOverline,m_bStrikeout,m_bTopline,m_bBottomline);		
		
    }
}



void XAP_UnixHildonDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	GtkWidget *pTopLevel = (static_cast<XAP_UnixFrameImpl*> (pFrame->getFrameImpl()))->getTopLevelWindow();
	PangoAttrList *default_list = pango_attr_list_new();	

	m_Widget = GTK_WIDGET(hildon_font_selection_dialog_new(GTK_WINDOW(gtk_widget_get_parent(GTK_WIDGET(pTopLevel))),
					    	NULL));		
	
	gtk_widget_show_all(GTK_WIDGET(m_Widget));
	
	m_answer = a_CANCEL;
	
	m_doneFirstFont = true;
	
	fillFontInfo(default_list);
	
	hildon_font_selection_dialog_set_font(HILDON_FONT_SELECTION_DIALOG(m_Widget), default_list);
	
	if (gtk_dialog_run(GTK_DIALOG(m_Widget)) == GTK_RESPONSE_OK)
	{
		
		default_list = hildon_font_selection_dialog_get_font ( HILDON_FONT_SELECTION_DIALOG ( m_Widget ) );		
		loadFontInfo(default_list);
		m_answer = a_OK;			
		
		//TODO
		//addOrReplaceVecProp("bgcolor",static_cast<XML_Char *>(buf_color));
	}	
	gtk_widget_destroy(GTK_WIDGET(m_Widget));
	m_doneFirstFont = false;
	
	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
							 ((getVal("font-family")) ? getVal("font-family") : ""),        ((m_bChangedFontFamily) ? "(chg)" : ""),
							 ((getVal("font-size")) ? getVal("font-size") : ""),            ((m_bChangedFontSize) ? "(chg)" : ""),
							 ((getVal("font-weight")) ? getVal("font-weight") : ""),        ((m_bChangedFontWeight) ? "(chg)" : ""),
							 ((getVal("font-style")) ? getVal("font-style") : ""),          ((m_bChangedFontStyle) ? "(chg)" : ""),
							 ((getVal("color")) ? getVal("color") : "" ),                           ((m_bChangedColor) ? "(chg)" : ""),
							 (m_bUnderline),                                                        ((m_bChangedUnderline) ? "(chg)" : ""),
							 (m_bStrikeout),                                                        ((m_bChangedStrikeOut) ? "(chg)" : "")));
	
}
