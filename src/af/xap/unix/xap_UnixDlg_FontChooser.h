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
	XAP_UnixDialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_FontChooser(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

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

	bool					getFont(XAP_UnixFont ** font);
	bool					getForegroundColor(UT_RGBColor * color);
	bool					getBackgroundColor(UT_RGBColor * color);	
	bool					getDecoration(bool * strikeout, bool * underline);
	bool					getSize(UT_uint32 * pointsize);
	bool					getEntryString(char ** string);
	GR_UnixGraphics * 		m_gc;

	bool					m_doneFirstFont;
	
protected:

	bool		 			m_blockUpdate;
	
	// careful, these must be in the order the
	// list box will show them (Windows order)
	typedef enum
	{	
	        LIST_STYLE_NONE = -1,
		LIST_STYLE_NORMAL = 0,
		LIST_STYLE_ITALIC,
		LIST_STYLE_BOLD,
		LIST_STYLE_BOLD_ITALIC
	} listStyle;
	
	// these are Glade helper or Glade generated functions
	GtkWidget * 			get_widget(GtkWidget * widget, gchar * widget_name);
	void 					set_notebook_tab(GtkWidget * notebook, gint page_num, GtkWidget * widget);
	virtual GtkWidget *             constructWindow(void);
	GtkWidget *                     constructWindowContents(GtkObject *);

	// a temporary font to hold dynamically allocated "rented"
	// fonts between style changes
	XAP_UnixFontHandle * 	m_lastFont;

	// parent frame
	XAP_UnixFrame *			m_pUnixFrame;
};

#endif /* XAP_UNIXDIALOG_FONTCHOOSER_H */
