/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef AP_QNXDialog_PageSetup_H
#define AP_QNXDialog_PageSetup_H

#include "ap_Dialog_PageSetup.h"

class AP_QNXDialog_PageSetup : public AP_Dialog_PageSetup
{
public:
	AP_QNXDialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_PageSetup (void);

	virtual void runModal (XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void event_OK (void);
	virtual void event_Cancel (void);
	virtual void event_WindowDelete (void);

	virtual void event_PageSizeChanged ();
	virtual void event_PageUnitsChanged ();
	virtual void event_MarginUnitsChanged ();

 protected:
	// construction functions
	virtual PtWidget_t * _constructWindow (void);

	// pointers to all the action items
	PtWidget_t * m_window;
	PtWidget_t * m_buttonOK;
	PtWidget_t * m_buttonCancel;

 private:
	// pointers to widgets that we may need to query
	PtWidget_t * m_optionPageSize;
	PtWidget_t * m_entryPageWidth;
	PtWidget_t * m_entryPageHeight;
	PtWidget_t * m_optionPageUnits;
	PtWidget_t * m_radioPagePortrait;
	PtWidget_t * m_radioPageLandscape;
	PtWidget_t * m_spinPageScale;

	PtWidget_t * m_optionMarginUnits;
	PtWidget_t * m_spinMarginTop;
	PtWidget_t * m_spinMarginBottom;
	PtWidget_t * m_spinMarginLeft;
	PtWidget_t * m_spinMarginRight;
	PtWidget_t * m_spinMarginHeader;
	PtWidget_t * m_spinMarginFooter;

	UT_Vector    m_vecunits;
	UT_Vector	 m_vecsize;

	int done;
};

#endif // AP_QNXDialog_PageSetup_H
