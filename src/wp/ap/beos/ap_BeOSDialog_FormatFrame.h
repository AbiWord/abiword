/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Daniel Furrer
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

#ifndef AP_BEOSDIALOG_FORMATFRAME_H
#define AP_BEOSDIALOG_FORMATFRAME_H

#include "ap_Dialog_FormatFrame.h"

class BWindow;

/*****************************************************************/

class AP_BeOSDialog_FormatFrame: public AP_Dialog_FormatFrame
{
public:
	AP_BeOSDialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_BeOSDialog_FormatFrame(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			runModeless(XAP_Frame *pFrame);
	virtual void			activate();
	virtual void			destroy();
	virtual void			setSensitivity(bool onoff);
	virtual void			setBorderThicknessInGUI(UT_UTF8String & sThick) { UT_ASSERT(UT_NOT_IMPLEMENTED); };

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	
	
private:
	BWindow *	_constructWindow();
	BWindow *	m_mainWindow;
};

#endif /* AP_BEOSDIALOG_FORMATFRAME_H */
