/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef XAP_DIALOG_STUB_H
#define XAP_DIALOG_STUB_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

class XAP_Frame;

/*
	This is not a real dialog.  It is not linked into the application. 
	Neither are its platform-specific subclasses.  The only reason 
	any of these files exist is to make sure that each of the 
	platform-specific stubs will always compile.  

	That way, when those stub files are cloned and renamed to make 
	stubs for other (real) dialogs, we know that they'll compile and 
	not break the build.  
	
	If you're creating a new XP dialog, ignore this file.  You'll be 
	*much* better off stealing code from another existing dialog which 
	does something real.  
*/
class XAP_Dialog_Stub : public XAP_Dialog_NonPersistent
{
public:
	XAP_Dialog_Stub(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Stub(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;

	XAP_Dialog_Stub::tAnswer		getAnswer(void) const;
	
protected:
	
	XAP_Dialog_Stub::tAnswer		m_answer;
};

#endif /* XAP_DIALOG_STUB_H */
