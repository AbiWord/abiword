/* AbiSource Application Framework
 * Copyright (C) 1998-2002 AbiSource, Inc.
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

#ifndef WITH_PANGO
#include "xap_UnixFontManager.h"
#else
#include "xap_PangoFontManager.h"
#endif

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
	void                    underlineChanged(void);
	void                    overlineChanged(void);
	void                    strikeoutChanged(void);
	void                    hiddenChanged(void);
	void                    transparencyChanged(void);
	void 					updatePreview(void);
	void                    fontRowChanged(void);
	void                    styleRowChanged(void);
	void                    sizeRowChanged(void);
	void                    fgColorChanged(void);
	void                    bgColorChanged(void);

	// the state of what data is hidden and what is public is
	// pretty grave here.
#ifndef WITH_PANGO
	XAP_UnixFontManager * 	m_fontManager;
#else
	const XAP_PangoFontManager * 	m_fontManager;
#endif
	GtkWidget * 			m_fontList;
	GtkWidget * 			m_styleList;
	GtkWidget * 			m_sizeList;
	GtkWidget * 			m_checkStrikeOut;
	GtkWidget *				m_checkUnderline;
	GtkWidget *				m_checkOverline;
	GtkWidget *                             m_checkHidden;
	GtkWidget *				m_checkTransparency;
	GtkWidget *				m_colorSelector;
	GtkWidget *				m_bgcolorSelector;
	GtkWidget * 			m_preview;
	bool					getEntryString(char ** string);
	GR_UnixGraphics * 		m_gc;

	bool		 			m_blockUpdate;
	bool		 			m_doneFirstFont;

protected:
	
	typedef enum
	  {
	    BUTTON_OK,
	    BUTTON_CANCEL
	  } ResponseId ;

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
	virtual GtkWidget *             constructWindow(void);
	GtkWidget *                     constructWindowContents(GtkWidget *);

	// a temporary font to hold dynamically allocated "rented"
	// fonts between style changes
#ifndef WITH_PANGO
	XAP_UnixFontHandle * 	m_lastFont;
#else
	PangoFont *             m_lastFont;
#endif

	// parent frame
	XAP_UnixFrame *			m_pUnixFrame;
	gdouble m_currentFGColor[4];
	gdouble m_currentBGColor[4];
	gdouble m_funkyColor[4];


};

#endif /* XAP_UNIXDIALOG_FONTCHOOSER_H */
