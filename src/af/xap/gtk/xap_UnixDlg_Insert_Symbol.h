/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2019 Hubert Figuière
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

#ifndef XAP_UNIXDIALOG_INSERT_SYMBOL_H
#define XAP_UNIXDIALOG_INSERT_SYMBOL_H

#include <list>
#include <string>

#include "xap_UnixDialog.h"
#include "xap_Dlg_Insert_Symbol.h"
#include <gdk/gdkkeysyms.h>

#define DEFAULT_UNIX_SYMBOL_FONT "Symbol"

class XAP_Frame;
class GR_CairoGraphics;

/*****************************************************************/

class XAP_UnixDialog_Insert_Symbol
	: public XAP_Dialog_Insert_Symbol
	, public XAP_UnixDialog
{
public:
	XAP_UnixDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Insert_Symbol(void);

	virtual void			runModal(XAP_Frame * pFrame) override;
	virtual void			runModeless(XAP_Frame * pFrame) override;
	virtual void			notifyActiveFrame(XAP_Frame *pFrame) override;
	virtual void			notifyCloseFrame(XAP_Frame * /*pFrame*/) override {};
	virtual void			destroy(void) override;
	virtual void			activate(void) override;

	void			event_Insert(void);
	void			event_WindowDelete(void);
	void			New_Font(void);
	void			New_Row(void);
	void 			Scroll_Event (int direction);
	void			setSymbolMap_size (UT_uint32 width, UT_uint32 height);


	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	typedef enum
		{
		  BUTTON_INSERT,
		  BUTTON_CLOSE = GTK_RESPONSE_CLOSE
		} ResponseId ;

	// callbacks can fire these events
	void			SymbolMap_exposed( void);
	void			Symbolarea_exposed( void);
	void			SymbolMap_clicked(GdkEvent * event);
	void			CurrentSymbol_clicked(GdkEvent *event);
	gboolean		Key_Pressed(GdkEventKey * e);

private:

	GtkWidget * _constructWindow(void);
	void        _getGlistFonts(std::list<std::string> & glFonts);
	GtkWidget * _createComboboxWithFonts (void);
	void        _connectSignals (void);
	void        _setScrolledWindow (void);

	// pointers to widgets we need to query/set
	GtkWidget * m_SymbolMap;
	GtkWidget * m_fontcombo;
	GtkAdjustment * m_vadjust;
	std::list<std::string> m_InsertS_Font_list;

	// private construction functions
	GtkWidget * _previewNew(int w, int h);

	GtkWidget * m_areaCurrentSym;
	GR_CairoGraphics * m_unixGraphics;
	GR_CairoGraphics * m_unixarea;
	UT_uint32 m_ix;
	UT_uint32 m_iy;
};

#endif /* XAP_UNIXDIALOG_INSERT_SYMBOL_H */
