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


#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

#include <gtk/gtk.h>
#include "ap_Frame.h"
#include "ut_vector.h"
class AP_UnixApp;
class ev_UnixKeyboard;
class EV_UnixMouse;
class EV_UnixMenu;

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** unix-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class AP_UnixFrame : public AP_Frame
{
public:
	AP_UnixFrame(AP_UnixApp * app);
	AP_UnixFrame(AP_UnixFrame * f);
	~AP_UnixFrame(void);

	virtual UT_Bool				initialize(void);
	virtual	AP_Frame *			cloneFrame(void);
	virtual UT_Bool				loadDocument(const char * szFilename);
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);
	virtual UT_Bool				updateTitle(void);

	GtkWidget *					getTopLevelWindow(void) const;
	GtkWidget *					getVBoxWidget(void) const;
	EV_UnixMouse *				getUnixMouse(void);
	ev_UnixKeyboard *			getUnixKeyboard(void);
	
protected:
	void						_createTopLevelWindow(void);
	UT_Bool						_showDocument(void);
	static void					_scrollFunc(void * pData, UT_sint32 xoff, UT_sint32 yoff);

	// TODO see why ev_UnixKeyboard has lowercase prefix...
	AP_UnixApp *				m_pUnixApp;
	ev_UnixKeyboard *			m_pUnixKeyboard;
	EV_UnixMouse *				m_pUnixMouse;
	EV_UnixMenu *				m_pUnixMenu;
	UT_Vector					m_vecUnixToolbars;
	
	GtkWidget *					m_wTopLevelWindow;
	GtkWidget *					m_wVBox;

	GtkAdjustment *				m_pVadj;
	GtkAdjustment *				m_pHadj;
	GtkWidget *					m_hScroll;
	GtkWidget *					m_vScroll;
	GtkWidget *					m_dArea;
	GtkWidget *					m_table;
	GtkWidget * 				m_wSunkenBox;
};

#endif /* AP_UNIXFRAME_H */
