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

#ifndef AP_UNIXDIALOG_MESSAGEBOX_H
#define AP_UNIXDIALOG_MESSAGEBOX_H

#include "xap_UnixFrame.h"
#include "xap_Dialog_MessageBox.h"
#include <glib.h>
class AP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_MessageBox : public AP_Dialog_MessageBox
{
public:
	AP_UnixDialog_MessageBox(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_UnixDialog_MessageBox(void);

	virtual void			runModal(AP_Frame * pFrame);

	static AP_Dialog *		static_constructor(AP_DialogFactory *, AP_Dialog_Id id);

	// must let static callbacks read our bindings
	UT_Vector * 		_getBindingsVector();
	void 				_setAnswer(AP_Dialog_MessageBox::tAnswer answer);
		
protected:
	AP_UnixFrame *			m_pUnixFrame;
	UT_Vector 				m_keyBindings;

	struct keyBinding
	{
		guint key;
		AP_Dialog_MessageBox::tAnswer answer;
	};

    // this should probably go in a base class, but the Unix dialogs don't inherit
    // from a common Unix dialog base class.  That kinda sucks.
	void _centerWindow(AP_Frame * parent, GtkWidget * child);

	void _bindKey(guint key, AP_Dialog_MessageBox::tAnswer answer);
	
};

#endif /* AP_UNIXDIALOG_MESSAGEBOX_H */
