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

#ifndef AP_BEOSSTATUSBAR_H
#define AP_BEOSSTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include "ut_types.h"
#include "ap_StatusBar.h"
#include "gr_BeOSGraphics.h"
#include "be_GRDrawView.h"

class XAP_Frame;

// I want to make this the height of a scroll bar
// (and not have is as a #define...)
#define STATUS_BAR_HEIGHT 20

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_BeOSStatusBar : public AP_StatusBar
{
public:
	AP_BeOSStatusBar(XAP_Frame * pFrame);
	virtual ~AP_BeOSStatusBar(void);

	virtual void		setView(AV_View * pView);
	be_GRDrawView *				createWidget(BRect r);

protected:
	be_GRDrawView *				m_wStatusBar;
	
};

#endif /* AP_BEOSSTATUSBAR_H */
