/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2002 AbiSource, Inc.
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

#ifndef XAP_UNIXDIALOG_HTMLOPTIONS_H
#define XAP_UNIXDIALOG_HTMLOPTIONS_H

#include "ut_types.h"

#include "xap_UnixDialogHelper.h"
#include "xap_Dialog.h"

#include "xap_Dlg_HTMLOptions.h"

class XAP_Frame;

class XAP_UnixDialog_HTMLOptions : public XAP_Dialog_HTMLOptions
{
public:
	XAP_UnixDialog_HTMLOptions (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	virtual ~XAP_UnixDialog_HTMLOptions (void);

	virtual void			runModal (XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor (XAP_DialogFactory *, XAP_Dialog_Id id);

	void			toggle_Is4 ();
	void			toggle_AbiWebDoc ();
	void			toggle_DeclareXML ();
	void			toggle_AllowAWML ();
	void			toggle_EmbedCSS ();
	void			toggle_EmbedImages ();

	void			refreshStates ();

 private:
	void			event_OK (void);
	void			event_SaveSettings (void);
	void			event_RestoreSettings (void);
	void			event_Cancel (void);

	GtkWidget *		_constructWindow (void);

	GtkWidget *		m_windowMain;

	GtkWidget *		m_wIs4;
	GtkWidget *		m_wAbiWebDoc;
	GtkWidget *		m_wDeclareXML;
	GtkWidget *		m_wAllowAWML;
	GtkWidget *		m_wEmbedCSS;
	GtkWidget *		m_wEmbedImages;
};

#endif /* XAP_UNIXDIALOG_HTMLOPTIONS_H */



