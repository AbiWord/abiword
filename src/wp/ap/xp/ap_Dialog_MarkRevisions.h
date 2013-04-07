/* AbiWord
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef AP_DIALOG_MARKREVISIONS_H
#define AP_DIALOG_MARKREVISIONS_H

#include "xap_Dialog.h"
#include "pd_Document.h"
#include "xap_Strings.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_MarkRevisions : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
			     XAP_Dialog_Id id);
	virtual ~AP_Dialog_MarkRevisions(void);

	virtual void  runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK=0, a_CANCEL=1 } tAnswer;

	tAnswer	      getAnswer(void) const;
	void          setAnswer(tAnswer a);

	void          setDocument(PD_Document * pDoc) {m_pDoc = pDoc;}
	void          forceNew() {m_bForceNew = true;}

	const char *        getTitle();
	const char *        getRadio2Label();
	const char *        getComment2Label();


	/* the caller is responsible for freeing the pointers returned by
	   the following two functions (use FREEP)
	*/
	char *        getRadio1Label();
	char *        getComment1(bool utf8 = false);

	void          setComment2(const char * pszComment);
	void          addRevision();
	bool		  isRev();

protected:
	AP_Dialog_MarkRevisions::tAnswer	m_answer;

private:
	void          _initRevision();

	PD_Document *                       m_pDoc;
	UT_UTF8String *                     m_pComment2;
	const XAP_StringSet *               m_pSS;
	const AD_Revision *                 m_pRev;
	bool                                m_bForceNew;
};

#endif /* AP_DIALOG_MARKREVISIONS_H */
