/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_UNIXDIALOG_FONTCHOOSER_H
#define XAP_UNIXDIALOG_FONTCHOOSER_H

#include "xap_App.h"
#include "xap_Dlg_FontChooser.h"
#include "ut_misc.h"

class XAP_Frame;
class GR_CairoGraphics;

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
	void                    subscriptChanged(void);
	void                    superscriptChanged(void);
	void                    strikeoutChanged(void);
	void                    hiddenChanged(void);
	void                    transparencyChanged(void);
	void 					updatePreview(void);
	void                    fontRowChanged(void);
	void                    styleRowChanged(void);
	void                    sizeRowChanged(void);
	void                    fgColorChanged(void);
	void                    bgColorChanged(void);
	void                    textTransformChanged(void);

	// the state of what data is hidden and what is public is
	// pretty grave here.
	GtkWidget * 			m_fontList;
	GtkWidget * 			m_styleList;
	GtkWidget * 			m_sizeList;
	GtkWidget * 			m_checkStrikeOut;
	GtkWidget *				m_checkUnderline;
	GtkWidget *				m_checkOverline;
	GtkWidget *				m_checkHidden;
	GtkWidget *				m_checkTransparency;
	GtkWidget *				m_checkSubScript;
	guint					m_iSubScriptId;
	GtkWidget *				m_checkSuperScript;
	guint					m_iSuperScriptId;
	GtkWidget *				m_colorSelector;
	GtkWidget *				m_bgcolorSelector;
	GtkWidget * 			m_preview;
	bool					getEntryString(char ** string);
	GR_CairoGraphics * 		m_gc;

	bool		 			m_blockUpdate;
	bool		 			m_doneFirstFont;

protected:

	// Gtk sets up escape key to close a GtkDialog only when
	// the one of the button responses is GTK_RESPONSE_CANCEL
	typedef enum
	  {
	    BUTTON_OK = GTK_RESPONSE_OK,
	    BUTTON_CANCEL = GTK_RESPONSE_CANCEL
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

	virtual GtkWidget *             constructWindow(void);
	GtkWidget *                     constructWindowContents(GtkWidget *);

	// parent frame
	XAP_Frame *			m_pFrame;
	GdkRGBA m_currentFGColor;
	GdkRGBA m_currentBGColor;
	GdkRGBA m_funkyColor;
	bool m_currentBGColorTransparent;
};

#endif /* XAP_UNIXDIALOG_FONTCHOOSER_H */
