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

#ifndef XAP_DIALOG_ENCODING_H
#define XAP_DIALOG_ENCODING_H

#include "ut_types.h"
#include "ut_xml.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Dialog.h"
#include "ut_Encoding.h"

/********************************************************************
INSTRUCTIONS FOR DESIGN OF THE PLATFORM VERSIONS OF THIS DIALOGUE

(1)	implement runModal(); at the moment we display a single listbox

(2)	m_iEncCount will tell you how many list entries there will be; 
	the encoding strings are then in m_ppEncodings (already sorted)

(3)	use _setEncoding() to set the member variables in response
	to the user selection when the dialog is closing.
*********************************************************************/



class XAP_Dialog_Encoding : public XAP_Dialog_NonPersistent
{
public:
	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;
	
	XAP_Dialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Encoding(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	void							setEncoding(const XML_Char * pEncoding);
	const XML_Char *				getEncoding() const;
	XAP_Dialog_Encoding::tAnswer	getAnswer(void) const;
	
protected:
	void							_setEncoding(const XML_Char * pEnc);

	void                            _setAnswer (XAP_Dialog_Encoding::tAnswer answer)
		{ m_answer = answer; };
	void                            _setSelectionIndex (UT_uint32 index)
		{ m_iSelIndex = index; };
	UT_uint32                       _getSelectionIndex () const
		{ return m_iSelIndex; };

private:
	XAP_Dialog_Encoding::tAnswer	m_answer;

	// the following keeps the string that the user sees in the dialogue; this is locale-dependent
	const XML_Char *				m_pDescription;
	// this keeps the actual encoding string corresponding to m_pDescription
	const XML_Char *				m_pEncoding;
	UT_Encoding *					m_pEncTable;
	const XML_Char **				m_ppEncodings;
	UT_uint32						m_iEncCount;
	UT_uint32						m_iSelIndex;
};
#endif /* XAP_DIALOG_ENCODING_H */


