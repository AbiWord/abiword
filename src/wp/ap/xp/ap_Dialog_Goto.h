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

#ifndef AP_DIALOG_FILE_H
#define AP_DIALOG_FILE_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"

/*****************************************************************
** This is the base-class for the Replace 
*****************************************************************/

class AP_Dialog_Goto : public AP_Dialog_FramePersistent
{
public:
	AP_Dialog_Goto(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_Goto(void);

	virtual void				useStart(void);
	virtual void				runModal(AP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	// these are kinda screwy now, but we never return anything but on
	// "cancel" or "close"
	typedef enum { a_VOID, a_FIND_NEXT, a_REPLACE, a_REPLACE_ALL, a_CANCEL }	tAnswer;

    AP_Dialog_Goto::tAnswer	 	getAnswer(void) const;

	// These are called from edit methods or from dialogs
	// to set or read the variables in the current
	// instance of the dialog.  These do not read the persistent
	// values.
	UT_Bool						setView(AV_View * view);
	AV_View * 					getView(void) const;
	
	UT_Bool						setPageNumberString(const UT_UCSChar * string);
	UT_UCSChar *				getPageNumberString(void);

	// Action functions... set data using the accessors
	// above and call one of these.
	UT_Bool						gotoPage(void);
	
 protected:

	// These are the persistent dialog data items,
	// which are carefully read and set by useStart()
	// and useEnd(), and not by the accessors.
	// TODO add 'persist' to variable names to help us
	// TODO identify them as such.  remove the leading
	// TODO '_'.
	UT_UCSChar *			_m_pageNumberString; 

	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.  Note that the buffers
	// these may point to are destroyed when useEnd()
	// is done storing them away
	FV_View * 				m_pView;
	UT_UCSChar *			m_pageNumberString; 

	// These are also "current use" dialog data item,
	// but they're not user-settable; they are set
	// on conditions that action functions or other
	// non-accessor methods are invoked.
	UT_Bool					m_didSomething;

	// Message boxes for events during search
	AP_Frame *				m_pFrame;

	void					_messageNoSuchPage(UT_uint32 pageNum = 0);
	void 					_messageBox(char * message);
	
	// is this used in a non-persistent dialog like this?
	tAnswer					m_answer;
};

#endif /* AP_DIALOG_REPLACE_H */
