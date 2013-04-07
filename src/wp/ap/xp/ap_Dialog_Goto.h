/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef AP_DIALOG_GOTO_H
#define AP_DIALOG_GOTO_H

#include <string>

#include "xap_Dialog.h"
#include "fv_View.h"
#include "pd_DocumentRDF.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_Goto : public XAP_Dialog_Modeless
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
  	FV_View * 					getView() const;
	void						setActiveFrame(XAP_Frame *pFrame);
	static const char **        getJumpTargets();
	PD_DocumentRDFHandle        getRDF();

	UT_sint32					getExistingBookmarksCount() const;
	const std::string &			getNthExistingBookmark(UT_sint32 n) const;
	/** Perform the Goto with a page #, line # or bookmark name
	 * @param target the target
	 * @param value the target value
	 */
	void                        performGoto(AP_JumpTarget target, const char *value) const;
	std::string                 performGotoNext(AP_JumpTarget target, UT_sint32 idx) const;
	std::string                 performGotoPrev(AP_JumpTarget target, UT_sint32 idx) const;
protected:
	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.
  	FV_View * 					m_pView;
	static const char *               s_pJumpTargets[];

	// is this used in a modeless dialog like this?
	tAnswer						m_answer;
	char						m_WindowName[100];
	void						_setupJumpTargets();
};

#endif /* AP_DIALOG_GOTO_H */
