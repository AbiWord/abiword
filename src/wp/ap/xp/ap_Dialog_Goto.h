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

#ifndef AP_DIALOG_GOTO_H
#define AP_DIALOG_GOTO_H

#include "xap_Dialog.h"

class FV_View;
class XAP_Frame;

class AP_Dialog_Goto : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Goto();

	// these are kinda screwy now, but we never return anything but on
	// "cancel" or "close"
	typedef enum {
		a_CLOSE
	} tAnswer;

	AP_Dialog_Goto::tAnswer		getAnswer() const;
	void						ConstructWindowName();
	// These are called from edit methods or from dialogs
	// to set or read the variables in the current
	// instance of the dialog.  These do not read the persistent
	// values.
  	bool						setView(FV_View * view);
  	FV_View * 					getView();
	void						setActiveFrame(XAP_Frame *pFrame);
	static char **              getJumpTargets(); // TODO: Change to UT_UCSChar

protected:
	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.
  	FV_View * 					m_pView;
	static char *               s_pJumpTargets[];

	// is this used in a modeless dialog like this?
	tAnswer						m_answer;
	char						m_WindowName[100];
	void						_setupJumpTargets();
};

#endif /* AP_DIALOG_GOTO_H */
