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

#ifndef AP_Dialog_Insert_Symbol_H
#define AP_Dialog_Insert_Symbol_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xap_Draw_Symbol.h"


#include "xav_View.h"

class XAP_Frame;

class AP_Dialog_Insert_Symbol : public XAP_Dialog_AppPersistent
{
 public:
	AP_Dialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Insert_Symbol(void);

	virtual void     useStart(void);
	virtual void     useEnd(void);
	virtual void	 runModal(XAP_Frame * pFrame) = 0;
	// read these back
	UT_UCSChar	     getInsertedSymbol(void);
	char *     getInsertedFont(void);
	typedef enum { a_OK, a_CANCEL} tAnswer;
	AP_Dialog_Insert_Symbol::tAnswer    getAnswer(void) const;
	void setView(AV_View *p_View)
	{
		m_pCurrentView = p_View;
	}

 protected:
	// handle the XP-job of drawing our symbols.
	void   _updateDrawSymbol();

	// handle the XP-job of laying out  our matrix of symbols
	void   _createSymbolFromGC(GR_Graphics * gc, UT_uint32 width,
							   UT_uint32 height);

	// handle the XP-job of drawing our symbol onto the preview area.
	void   _updateDrawSymbolarea( UT_UCSChar c, UT_UCSChar p);

	// handle the XP-job of constructing the preview area
	void   _createSymbolareaFromGC(GR_Graphics * gc, UT_uint32 width,
								   UT_uint32 height);
	void   _onInsertButton();

	// This function returns the current Symbol Map class
	XAP_Draw_Symbol *  _getCurrentSymbolMap();

	// This function lays out the symbol matrix
	XAP_Draw_Symbol *  m_DrawSymbol;

	// This variable stores the current symbol font
	GR_Font *		   m_Insert_Symbol_font;

	// This is character selected.
	UT_UCSChar	       m_Inserted_Symbol;
	AP_Dialog_Insert_Symbol::tAnswer   m_answer;

	AV_View	*			m_pCurrentView;
};

#endif /* XAP_DIALOG_ZOOM_H */
