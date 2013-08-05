/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2013 Serhat Kiyak <serhatkiyak91@gmail.com>
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

#ifndef AP_QTSTATUSBAR_H
#define AP_QTSTATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include <QStatusBar>
#include <QWidget>

#include "ut_types.h"
#include "ap_StatusBar.h"
class XAP_Frame;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_QtStatusBar : public AP_StatusBar
{
public:
	AP_QtStatusBar(XAP_Frame * pFrame);
	virtual ~AP_QtStatusBar(void);

	QStatusBar *		createWidget(void);
	virtual void		hideProgressBar(void);

protected:
	QStatusBar *		m_wStatusBar;
	QWidget *		m_wProgressFrame;
};

#endif /* AP_QTSTATUSBAR_H */
