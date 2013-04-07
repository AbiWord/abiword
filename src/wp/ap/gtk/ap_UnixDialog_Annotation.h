/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

#ifndef AP_UNIXDIALOG_ANNOTATION_H
#define AP_UNIXDIALOG_ANNOTATION_H

#include "xap_UnixDialogHelper.h"
#include "ap_Dialog_Annotation.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Annotation: public AP_Dialog_Annotation
{
 public:
	AP_UnixDialog_Annotation(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Annotation(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:

	void eventCancel () ;
	void eventOK () ;
	void eventApply () ;

	virtual GtkWidget * _constructWindow () ;

 private:

 	GtkWidget * m_windowMain;
	GtkWidget * m_entryTitle;
	GtkWidget * m_entryAuthor;
	GtkWidget * m_textDescription;
};

#endif /* AP_UNIXDIALOG_ANNOTATION_H */
