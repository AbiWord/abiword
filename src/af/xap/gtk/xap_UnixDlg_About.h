/*
 * Copyright (C) 2006 Rob Staudinger <robert.staudinger@gmail.com>
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

#ifndef XAP_UNIXDIALOG_ABOUT_H
#define XAP_UNIXDIALOG_ABOUT_H

#include "xap_Dlg_About.h"

class XAP_Frame;

class XAP_UnixDialog_About: public XAP_Dialog_About
{
public:
	XAP_UnixDialog_About(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_About(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void			runModal(XAP_Frame * pFrame);

};

#endif /* XAP_UNIXDIALOG_ABOUT_H */
