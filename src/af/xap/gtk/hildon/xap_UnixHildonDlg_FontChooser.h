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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */



#ifndef XAP_UNIXHILDONDIALOG_FONTCHOOSER_H
#define XAP_UNIXHILDONDIALOG_FONTCHOOSER_H

#include "xap_App.h"
//#include "xap_UnixFontManager.h"
#include "xap_Dlg_FontChooser.h"
#include "ut_misc.h"

class XAP_Frame;

/*****************************************************************/

class XAP_UnixHildonDialog_FontChooser : public XAP_Dialog_FontChooser
{
public:
	XAP_UnixHildonDialog_FontChooser(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixHildonDialog_FontChooser(void);

	static XAP_Dialog * 	static_constructor(XAP_DialogFactory * pFactory,
												XAP_Dialog_Id id);

	virtual void			runModal(XAP_Frame * pFrame);

	// the state of what data is hidden and what is public is
	// pretty grave here.
	//XAP_UnixFontManager * 	m_fontManager;

	GtkWidget* 				m_Widget;

	bool		 			m_blockUpdate;
	bool		 			m_doneFirstFont;

protected:


	virtual void 			fillFontInfo();
	virtual void			loadFontInfo();

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

	// a temporary font to hold dynamically allocated "rented"
	// fonts between style changes
	//XAP_UnixFontHandle * 	m_lastFont;

	// parent frame
	XAP_Frame *			m_pFrame;
	gdouble m_currentFGColor[4];
	gdouble m_currentBGColor[4];
	gdouble m_funkyColor[4];


};

#endif /* XAP_UNIXHILDONDIALOG_FONTCHOOSER_H
*/
