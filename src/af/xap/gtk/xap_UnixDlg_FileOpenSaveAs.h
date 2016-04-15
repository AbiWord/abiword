/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

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

#ifndef XAP_UNIXDIALOG_FILEOPENSAVEAS_H
#define XAP_UNIXDIALOG_FILEOPENSAVEAS_H
#include <gtk/gtk.h>
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Strings.h"
#include "ut_vector.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

class XAP_Frame;
class UT_ByteBuf;
/*****************************************************************/

class XAP_UnixDialog_FileOpenSaveAs : public XAP_Dialog_FileOpenSaveAs
{
public:
	XAP_UnixDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_FileOpenSaveAs(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	gint previewPicture ();

	void fileTypeChanged(GtkWidget * w);
	void onDeleteCancel (void);

protected:
	GdkPixbuf *            pixbufForByteBuf (UT_ByteBuf * pBB);
	GdkPixbuf *            _loadXPM(UT_ByteBuf * pBB);

	bool					_run_gtk_main(XAP_Frame * pFrame,
										  GtkWidget * filetypes_pulldown);
	void 					_notifyError_OKOnly(XAP_Frame * pFrame,
												XAP_String_Id sid);
	void 					_notifyError_OKOnly(XAP_Frame * pFrame,
												XAP_String_Id sid,
												const char * sz1);
	bool 				_askOverwrite_YesNo(XAP_Frame * pFrame,
												const char * fileName);

	GtkFileChooser * m_FC;
	GtkWidget * m_preview;
private:
	bool				m_bSave;
protected:
	XAP_Frame *			m_pFrame;
	std::string			m_finalPathnameCandidate;
private:
	GtkWidget *         m_wFileTypes_PullDown;

};

#endif /* XAP_UNIXDIALOG_FILEOPENSAVEAS_H */
