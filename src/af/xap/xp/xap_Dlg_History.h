/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#ifndef XAP_DIALOG_HISTORY_H
#define XAP_DIALOG_HISTORY_H

#include "xap_Dialog.h"
#include "ut_vector.h"

/*
    The dialogue consists of three segements (see PNG attached to
    commit message of Feb 5, 2004)

    Buttons: there are getButtonCount() buttons (currently 2) which
             should be localised by getButtonLabel(n); the 0-th button
             is the default one; count-1 button is the cancel button.

    Header section: the upper part of the dialogue; consists of
             getHeaderItemCount() pairs of labels, each pair on a
             separate row; the content of the labels is obtained
             by getHeaderLabel(n) and getHeaderValue(n)

    List control: a multicolumn list; the number of columns is
             getListColumnCount(), while the number of items (i.e.,
             rows) is getListItemCount(). Strings for each cell in the
             list are obtained via getListValue(row, column); the
             title for entire list comes from getListTitle().

             getListItemId(n) returns numerical id for a given row; if
             the user closes the dialogue in any other way than
             pressing Cancel, the numerical id for selected row should
             be passed to setSelectionId() before the dialogue closes;
             m_answer should be set as usual.

    Title for the dialogue window is obtained by getWindowLabel().
 */

class XAP_Frame;
class AD_Document;
class XAP_StringSet;

const UT_uint32 iHeaderItemCount = 6;
const UT_uint32 iButtonCount = 2;
const UT_uint32 iListColumnCount = 3;

class ABI_EXPORT XAP_Dialog_History : public XAP_Dialog_NonPersistent
{
  public:
	XAP_Dialog_History(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	virtual ~XAP_Dialog_History(void) {};

	virtual void runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL, a_SHOW } tAnswer;

	XAP_Dialog_History::tAnswer		getAnswer(void) const {return m_answer;};

	void         setDocument(AD_Document * pDoc) {m_pDoc = pDoc;}

	UT_uint32    getHeaderItemCount() const {return iHeaderItemCount;}
	const char * getHeaderLabel(UT_uint32 indx) const;
	char *       getHeaderValue(UT_uint32 indx) const;

	UT_uint32    getButtonCount() const {return iButtonCount;}
	const char * getButtonLabel(UT_uint32 indx) const;

	const char * getListTitle() const;
	UT_uint32    getListColumnCount() const {return iListColumnCount;}
	const char * getListHeader(UT_uint32 column) const;
	UT_uint32    getListItemCount() const;
	char *       getListValue(UT_uint32 item, UT_uint32 column) const;
	UT_uint32    getListItemId(UT_uint32 item) const;

	const char * getWindowLabel() const;

	UT_uint32    getSelectionId() const {return m_iId;}
	void         setSelectionId(UT_uint32 iId) {m_iId = iId;}

  protected:
	XAP_Dialog_History::tAnswer		m_answer;

  private:
	const AD_Document *   m_pDoc;
	const XAP_StringSet * m_pSS;
	UT_uint32             m_iId;
};

#endif /* XAP_DIALOG_HISTORY_H */
