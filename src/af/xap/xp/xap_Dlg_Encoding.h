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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_DIALOG_ENCODING_H
#define XAP_DIALOG_ENCODING_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
/* #include "ut_Encoding.h" */
/* #include "ut_assert.h" */
/* #include "ut_debugmsg.h" */

#include "xap_Dialog.h"

class UT_Encoding;

/********************************************************************
INSTRUCTIONS FOR DESIGN OF THE PLATFORM VERSIONS OF THIS DIALOGUE

(1)	implement runModal(); at the moment we display a single listbox

(2)	m_iEncCount will tell you how many list entries there will be;
	the encoding strings are then in m_ppEncodings (already sorted)

(3)	use _setEncoding() to set the member variables in response
	to the user selection when the dialog is closing.
*********************************************************************/



class ABI_EXPORT XAP_Dialog_Encoding : public XAP_Dialog_NonPersistent
{
public:
	typedef enum { a_OK, a_CANCEL, a_YES, a_NO }	tAnswer;

	XAP_Dialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Encoding(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;
	void							setEncoding(const gchar * pEncoding);
	const gchar *				getEncoding() const;
	XAP_Dialog_Encoding::tAnswer	getAnswer(void) const;

protected:
	void							_setEncoding(const gchar * pEnc);

	void                            _setAnswer (XAP_Dialog_Encoding::tAnswer answer)
		{ m_answer = answer; };
	void                            _setSelectionIndex (UT_uint32 index)
		{ m_iSelIndex = index; };
	UT_uint32                       _getSelectionIndex () const
		{ return m_iSelIndex; };
	const gchar **               _getAllEncodings () const
		{ return m_ppEncodings; };
	UT_uint32                       _getEncodingsCount() const
		{ return m_iEncCount; };

private:
	XAP_Dialog_Encoding::tAnswer	m_answer;

	// the following keeps the string that the user sees in the dialogue; this is locale-dependent
	const gchar *				m_pDescription;
	// this keeps the actual encoding string corresponding to m_pDescription
	const gchar *				m_pEncoding;
	UT_Encoding *					m_pEncTable;
	const gchar **				m_ppEncodings;
	UT_uint32						m_iEncCount;
	UT_uint32						m_iSelIndex;
};
#endif /* XAP_DIALOG_ENCODING_H */


