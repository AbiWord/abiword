/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_FONTCHOOSER_H
#define XAP_COCOADIALOG_FONTCHOOSER_H

#include "xap_App.h"
#include "xap_Dlg_FontChooser.h"
#include "ut_misc.h"

class XAP_CocoaFrame;
class GR_CocoaGraphics;

/*****************************************************************/

class XAP_CocoaDialog_FontChooser : public XAP_Dialog_FontChooser
{
public:
	XAP_CocoaDialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_CocoaDialog_FontChooser(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	void                    underlineChanged(void);
	void                    overlineChanged(void);
	void                    strikeoutChanged(void);
	void                    transparencyChanged(void);
	void 					updatePreview(void);
	void                    fontRowChanged(void);
	void                    styleRowChanged(void);
	void                    sizeRowChanged(void);
	void                    fgColorChanged(void);
	void                    bgColorChanged(void);

	// the state of what data is hidden and what is public is
	// pretty grave here.
//	XAP_CocoaFontManager * 	m_fontManager;
#if 0
	GtkWidget * 			m_fontList;
	GtkWidget * 			m_styleList;
	GtkWidget * 			m_sizeList;
	GtkWidget * 			m_checkStrikeOut;
	GtkWidget *				m_checkUnderline;
	GtkWidget *				m_checkOverline;
	GtkWidget *				m_checkTransparency;
	GtkWidget *				m_colorSelector;
	GtkWidget *				m_bgcolorSelector;
	GtkWidget * 			m_preview;
#endif
	bool					getEntryString(char ** string);
	GR_CocoaGraphics * 		m_gc;

	bool		 			m_blockUpdate;
	bool		 			m_doneFirstFont;

protected:
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

#if 0
	// these are Glade helper or Glade generated functions
	GtkWidget * 			get_widget(GtkWidget * widget, gchar * widget_name);
	void 					set_notebook_tab(GtkWidget * notebook, gint page_num, GtkWidget * widget);
	virtual GtkWidget *             constructWindow(void);
	GtkWidget *                     constructWindowContents(GtkObject *);

	// a temporary font to hold dynamically allocated "rented"
	// fonts between style changes
	XAP_CocoaFontHandle * 	m_lastFont;

	// parent frame
	XAP_CocoaFrame *			m_pCocoaFrame;
	gdouble m_currentFGColor[4];
	gdouble m_currentBGColor[4];
	gdouble m_funkyColor[4];
#endif

};

#endif /* XAP_COCOADIALOG_FONTCHOOSER_H */
