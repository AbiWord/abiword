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

#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "fl_BlockLayout.h"
#include "pt_Types.h"

/*****************************************************************
** This is the base-class for the Replace 
*****************************************************************/

class AP_Dialog_Replace : public AP_Dialog_FramePersistent
{
public:
	AP_Dialog_Replace(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_Replace(void);

	virtual void				useStart(void);
	virtual void				runModal(AP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	typedef enum { a_VOID, a_FIND_NEXT, a_REPLACE, a_REPLACE_ALL, a_CANCEL }	tAnswer;

    AP_Dialog_Replace::tAnswer	getAnswer(void) const;

	void						resetFind(void);
	void						setView(AV_View * view);
	UT_Bool						findNext(char * string);
	UT_Bool						findNextAndReplace(char * find, char * replace);
	
 protected:

	// GUI data (should this move do derived platform class, or
	// will it always be common at a high level)?  Also, is
	// UT_UCSChar annoying to work with at this level?
	UT_UCSChar *			m_findString; 
	UT_UCSChar *			m_replaceString;
	UT_Bool					m_matchCase;

	FV_View * 				m_pView;
	
	// is this used in a non-persistent dialog like this?
	tAnswer					m_answer;
};

#endif /* AP_DIALOG_REPLACE_H */
