/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_UNIXGNOMEDIALOG_WORDCOUNT_H
#define AP_UNIXGNOMEDIALOG_WORDCOUNT_H

#include "ap_UnixDialog_WordCount.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixGnomeDialog_WordCount: public AP_UnixDialog_WordCount
{
public:
	AP_UnixGnomeDialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixGnomeDialog_WordCount(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:
	// private construction functions
	virtual GtkWidget * _constructWindow(void);

	// pointers to widgets we need to query/set
	//	GtkWidget * m_windowMain;
	//	GtkWidget * m_buttonOK;
};

#endif /* AP_UNIXGNOMEDIALOG_WORDCOUNT_H */
