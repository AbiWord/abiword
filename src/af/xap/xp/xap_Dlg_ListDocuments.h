/* AbiSource Application Framework
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef XAP_DIALOG_LISTDOCS_H
#define XAP_DIALOG_LISTDOCS_H

#include "xap_Dialog.h"
#include "ut_assert.h"
#include "ut_vector.h"

class AD_Document;

/*!

    This dialogue lists documents that are open; by default
    the current document is excluded from the list. This can be
    changed by calling setIncludeActiveDocument(true) before the
    dialogue is displayed.

    PLATFORM NOTES
	--------------
    Platform subclasses need to provide the usual static_constructor()
    and runModal() methods.

    The dialogue contains a single column list which is filled by
    document names using _getDocumentCount() and
    _getNthDocumentName(). There is a heading text for the list,
    obtained through _getHeading();

	When the user selects a particular entry on the list,
	_setSelDocumentIndx() should be called to store the index of the
	selection (if the subclass sorts the list, it needs to ensure that
	the index stored is the original index of that entry).

    There should be two buttons: OK and Cancel; the text for the OK
    button needs to be obtained using _getOKButtonText().

	Title for the dialogue window is obtained through _getTitle().

	(this dialogue is very much like ListRevisions, except the list
	has only one column and the OK button label is not static)

*/

class ABI_EXPORT XAP_Dialog_ListDocuments : public XAP_Dialog_NonPersistent
{
  public:
	XAP_Dialog_ListDocuments(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_ListDocuments(void);

	virtual void					runModal(XAP_Frame * pFrame) = 0;

	typedef enum { a_OK, a_CANCEL }	tAnswer;

	XAP_Dialog_ListDocuments::tAnswer getAnswer(void) const {return m_answer;}

    AD_Document * 					  getDocument(void) const;

	void                              setIncludeActiveDoc(bool b);

  protected:
	void                       _setAnswer(XAP_Dialog_ListDocuments::tAnswer a){m_answer=a;}
	UT_sint32                  _getDocumentCount() {return m_vDocs.getItemCount();}
	const char *               _getNthDocumentName(UT_sint32 n) const;

	void                       _setSelDocumentIndx(UT_sint32 i);

	const char *               _getTitle() const;
	const char *               _getOKButtonText() const;
	const char *               _getHeading() const;

  private:

	void                              _init();

	XAP_Dialog_ListDocuments::tAnswer m_answer;

	UT_sint32						  m_ndxSelDoc;
	UT_Vector                         m_vDocs;
	bool                              m_bIncludeActiveDoc;
};

#endif /* XAP_DIALOG_WINDOWMORE_H */
