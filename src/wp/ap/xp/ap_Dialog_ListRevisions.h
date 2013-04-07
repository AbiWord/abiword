/* AbiWord
 * Copyright (C) 2002, 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef AP_DIALOG_LISTREVISIONS_H
#define AP_DIALOG_LISTREVISIONS_H

#include "xap_Dialog.h"
#include "pd_Document.h"
#include "xap_Strings.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_ListRevisions : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
			     XAP_Dialog_Id id);
	virtual ~AP_Dialog_ListRevisions(void);

	virtual void  runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK=0, a_CANCEL=1 } tAnswer;

	tAnswer	      getAnswer(void) const;
	void          setAnswer(tAnswer a);

	void          setDocument(PD_Document * pDoc) {m_pDoc = pDoc;}

	const char *        getTitle() const;
	const char *        getLabel1()const;
	const char *        getColumn1Label() const;
	const char *        getColumn2Label() const;
	const char *        getColumn3Label() const;

	UT_uint32           getItemCount() const;
	UT_uint32           getNthItemId(UT_uint32 n) const;

	/* the caller is responsible for freeing the pointer returned by
	   the following function (use FREEP) */
	char *              getNthItemText(UT_uint32 n, bool utf8 = false) const;

	const char *        getNthItemTime(UT_uint32 n) const;
    time_t              getNthItemTimeT(UT_uint32 n) const;

	UT_uint32           getSelectedId() const {return m_iId;}

protected:
	AP_Dialog_ListRevisions::tAnswer	m_answer;
	UT_uint32                           m_iId;

private:
	PD_Document *                       m_pDoc;
	const XAP_StringSet *               m_pSS;
};

#endif /* AP_DIALOG_TOGGLECASE_H */
