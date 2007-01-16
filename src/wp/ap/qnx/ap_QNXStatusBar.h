/* AbiWord
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

#ifndef AP_QNXSTATUSBAR_H
#define AP_QNXSTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include <Pt.h>
#include "ut_types.h"
#include "ap_StatusBar.h"
#include "gr_QNXGraphics.h"
class XAP_Frame;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_QNXStatusBar : public AP_StatusBar
{
public:
	AP_QNXStatusBar(XAP_Frame * pFrame);
	virtual ~AP_QNXStatusBar(void);

	virtual void		setView(AV_View * pView);
	PtWidget_t *			createWidget(void);

	virtual void		show(void);
	virtual void		hide(void);

protected:
	PtWidget_t *			m_wStatusBar;
};

#endif /* AP_QNXSTATUSBAR_H */
