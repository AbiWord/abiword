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

#ifndef XAP_UNIXGNOMEDIALOG_LANG_H
#define XAP_UNIXGNOMEDIALOG_LANG_H

#include "xap_UnixDlg_Language.h"

/*****************************************************************/

class XAP_UnixGnomeDialog_Language: public XAP_UnixDialog_Language
{
 public:
	XAP_UnixGnomeDialog_Language(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixGnomeDialog_Language(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:
	virtual GtkWidget * constructWindow(void);
 private:
};

#endif /* XAP_UNIXGNOMEDIALOG_LANG_H */
