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

#ifndef XAP_UNIXDIALOG_FONTCHOOSER_H
#define XAP_UNIXDIALOG_FONTCHOOSER_H

#include "xap_App.h"
#include "xap_UnixFontManager.h"
#include "xap_Dlg_FontChooser.h"
#include "ut_misc.h"

class XAP_UnixFrame;
class GR_UnixGraphics;

/*****************************************************************/

class XAP_UnixDialog_FontChooser : public XAP_Dialog_FontChooser
{
public:
	XAP_UnixDialog_FontChooser(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_UnixDialog_FontChooser(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

	void 					updatePreview(void);
	
	// the state of what data is hidden and what is public is
	// pretty grave here.
	XAP_UnixFontManager * 	m_fontManager;
	GtkWidget * 			m_fontList;
	GtkWidget * 			m_styleList;
	GtkWidget * 			m_sizeList;
	GtkWidget * 			m_checkStrikeOut;
	GtkWidget *				m_checkUnderline;
	GtkWidget *				m_colorSelector;
	GtkWidget * 			m_preview;

	UT_Bool					getFont(XAP_UnixFont ** font);
	UT_Bool					getForegroundColor(UT_RGBColor * color);
	UT_Bool					getBackgroundColor(UT_RGBColor * color);	
	UT_Bool					getDecoration(UT_Bool * strikeout, UT_Bool * underline);
	UT_Bool					getSize(UT_uint32 * pointsize);
	UT_Bool					getEntryString(char ** string);
	GR_UnixGraphics * 		m_gc;

protected:

	UT_Bool 			m_blockUpdate;

	// careful, these must be in the order the
	// list box will show them (Windows order)
	typedef enum
	{	
		LIST_STYLE_NORMAL = 0,
		LIST_STYLE_ITALIC,
		LIST_STYLE_BOLD,
		LIST_STYLE_BOLD_ITALIC
	} listStyle;
	
	// these are Glade helper or Glade generated functions
	GtkWidget * 			get_widget(GtkWidget * widget, gchar * widget_name);
	void 					set_notebook_tab(GtkWidget * notebook, gint page_num, GtkWidget * widget);
	GtkWidget * 			create_windowFontSelection(void);

	// a temporary font to hold dynamically allocated "rented"
	// fonts between style changes
	XAP_UnixFontHandle * 	m_lastFont;

	// parent frame
	XAP_UnixFrame *			m_pUnixFrame;
	XAP_App * 				m_pApp;
};

#endif /* XAP_UNIXDIALOG_FONTCHOOSER_H */
