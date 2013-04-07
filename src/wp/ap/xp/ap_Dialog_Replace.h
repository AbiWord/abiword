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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_DIALOG_REPLACE_H
#define AP_DIALOG_REPLACE_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "pt_Types.h"

class ABI_EXPORT AP_Dialog_Replace : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Replace(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Replace(void);

	//------------------------------------------------------------
	// All these are needed for a modeless dialog

	virtual void     useStart(void);
	virtual void     useEnd(void);
	virtual void	 runModal(XAP_Frame * pFrame) = 0;
	virtual void	 runModeless(XAP_Frame * pFrame) = 0;
        virtual void     destroy(void)=0;
        virtual void     activate(void)=0;
	void		 setActiveFrame(XAP_Frame *pFrame);
	virtual void		notifyActiveFrame(XAP_Frame *pFrame) = 0;

	typedef enum { find_FIND_NEXT, find_REPLACE, find_REPLACE_ALL } tFindType;
	typedef enum { a_VOID, a_FIND_NEXT, a_REPLACE, a_REPLACE_ALL, a_CANCEL }	tAnswer;
        void                                       ConstructWindowName(void);
	char *		getWindowName(void) { return m_WindowName; };

    AP_Dialog_Replace::tAnswer	getAnswer(void) const;

	// These are called from edit methods or from dialogs
	// to set or read the variables in the current
	// instance of the dialog.  These do not read the persistent
	// values.
	bool						setView(AV_View * view);
	AV_View * 					getView(void);
	FV_View * 					getFvView(void);

	void						setFindString(const UT_UCSChar * string);
	UT_UCSChar *				getFindString(void);

	void						setReplaceString(const UT_UCSChar * string);
	UT_UCSChar * 				getReplaceString(void);

	void						setMatchCase(bool match);
	bool						getMatchCase(void);

	void						setReverseFind( bool newValue);
	bool						getReverseFind(void);

	void						setWholeWord( bool newValue);
	bool						getWholeWord(void);

	// Action functions... set data using the accessors
	// above and call one of these.
	bool						findNext(void);
	bool						findPrev(void);
	bool						findReplaceReverse(void);
	bool						findReplace(void);
	bool 						findReplaceAll(void);

 protected:

	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.  Note that the buffers
	// these may point to are destroyed when useEnd()
	// is done storing them away
	FV_View * 					m_pView;
	UT_UCSChar *				m_findString;
	UT_UCSChar *				m_replaceString;

	// Message boxes for events during search
	XAP_Frame *					m_pFrame;

	void						_messageFinishedFind(void);
	void						_messageFinishedReplace(UT_uint32 numReplaced = 0);
	void 						_messageBox(const char * message);

	// is this used in a modeless dialog like this?
	tAnswer						m_answer;
	char						m_WindowName[100];

	// save a list of find a replace texts
	UT_GenericVector<UT_UCS4Char*>	m_findList;
	UT_GenericVector<UT_UCS4Char*>	m_replaceList;
	virtual void				_updateLists() = 0; // must be implemented in non-xp code

 private:
	// returns true when the internal list was changed
	bool						_manageList(UT_GenericVector<UT_UCS4Char*>* list, UT_UCSChar* string);
};

#endif /* AP_DIALOG_REPLACE_H */
