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

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"

class AP_Dialog_Goto : public AP_Dialog_FramePersistent
{
public:
	AP_Dialog_Goto(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_Goto(void);

	virtual void				useStart(void);
	virtual void				runModeless(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	// these are kinda screwy now, but we never return anything but on
	// "cancel" or "close"
	typedef enum
		{
			a_VOID,
			a_FIND_NEXT,
			a_REPLACE,
			a_REPLACE_ALL,
			a_CANCEL
		}
	tAnswer;

    AP_Dialog_Goto::tAnswer	 	getAnswer(void) const;

	// These are called from edit methods or from dialogs
	// to set or read the variables in the current
	// instance of the dialog.  These do not read the persistent
	// values.
	UT_Bool						setView(AV_View * view);
	AV_View * 					getView(void) const;

	UT_Bool						setTargetType(FV_JumpTarget target);
	FV_JumpTarget				getTargetType(void);
	
	UT_Bool						setTargetData(const UT_UCSChar * string);
	UT_UCSChar *				getTargetData(void);

	// Action functions... set data using the accessors
	// above and call one of these.
	UT_Bool						gotoTarget(void);
	
 protected:
	
	// These are the persistent dialog data items,
	// which are carefully read and set by useStart()
	// and useEnd(), and not by the accessors.
	FV_JumpTarget				persist_targetType;
	UT_UCSChar *				persist_targetData;
		
	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.  Note that the buffers
	// these may point to are destroyed when useEnd()
	// is done storing them away
	FV_View * 					m_pView;

	FV_JumpTarget				m_targetType;
	UT_UCSChar *				m_targetData;

	// These are also "current use" dialog data item,
	// but they're not user-settable; they are set
	// on conditions that action functions or other
	// non-accessor methods are invoked.
	UT_Bool						m_didSomething;

	// is this used in a modeless dialog like this?
	tAnswer						m_answer;
};

#endif /* AP_DIALOG_GOTO_H */
