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

#ifndef XAP_UNIXDIALOG_INSERT_SYMBOL_H
#define XAP_UNIXDIALOG_INSERT_SYMBOL_H

#include "xap_Dlg_Insert_Symbol.h"
#include "xap_UnixFontManager.h"
#include <gdk/gdkkeysyms.h>

static UT_uint32 xap_UnixDlg_Insert_Symbol_first = 0;
static UT_UCSChar m_CurrentSymbol;
static UT_UCSChar m_PreviousSymbol;

#define DEFAULT_UNIX_SYMBOL_FONT "Standard Symbols"

class XAP_UnixFrame;

/*****************************************************************/

class XAP_UnixDialog_Insert_Symbol: public XAP_Dialog_Insert_Symbol
{
public:
	XAP_UnixDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Insert_Symbol(void);

	virtual void			runModal(XAP_Frame * pFrame);

	virtual void			runModeless(XAP_Frame * pFrame);
        virtual void                    destroy( void);
        virtual void                    activate(void);
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			SymbolMap_exposed( void);
	virtual void			Symbolarea_exposed( void);
	virtual void			SymbolMap_clicked(GdkEvent * event);
	virtual void			Key_Pressed(GdkEventKey * e);
	virtual void            New_Font( void);
	virtual void			event_WindowDelete(void);
       
protected:

	GR_UnixGraphics	* 		m_unixGraphics;
	GR_UnixGraphics *       m_unixarea;

	// private construction functions
	GtkWidget * _constructWindow(void);

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	GtkWidget * m_SymbolMap;

	GtkWidget * m_areaCurrentSym;
	GtkWidget * m_fontcombo;
	
	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
	GList * m_InsertS_Font_list;
	gchar * m_fontlist[100];
	UT_uint32 m_Insert_Symbol_no_fonts;
};

#endif /* XAP_UNIXDIALOG_INSERT_SYMBOL_H */










