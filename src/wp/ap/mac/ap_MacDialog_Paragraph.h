/* AbiWord
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

#ifndef AP_MACDIALOG_PARAGRAPH_H
#define AP_MACDIALOG_PARAGRAPH_H

#include "ap_Dialog_Paragraph.h"

class XAP_MacFrame;

/*****************************************************************/

class AP_MacDialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_MacDialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_MacDialog_Paragraph(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	virtual void			setAnswer(AP_Dialog_Paragraph::tAnswer a) { m_answer = a; };

protected:
	virtual void			_syncControls(tControl changed, bool bAll = false);
	virtual void			_populateWindowData();
	
       	void _redrawPreview();
    #if 0
	void _spinnerChanged(class BTextControl* pTextBox);
	void _checkBoxChanged(class BCheckBox* pBox);
	void _menuChanged(uint32 ID , uint32 itemIndex);
	class ParagraphWin *newwin;
    #endif
};

#endif /* AP_MACDIALOG_PARAGRAPH_H */
