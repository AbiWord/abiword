/* AbiSource Program Utilities
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

#ifndef EV_QNXMENU_H
#define EV_QNXMENU_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Menu.h"
#include <Pt.h>

class AV_View;
class XAP_QNXApp;
class XAP_QNXFrame;


/*****************************************************************/

class EV_QNXMenu : public EV_Menu
{
public:
	EV_QNXMenu(XAP_QNXApp * pQNXApp,
				XAP_QNXFrame * pQNXFrame,
				const char * szMenuLayoutName,
				const char * szMenuLabelSetName);
	//virtual ~EV_QNXMenu(void);
	~EV_QNXMenu(void);

	UT_Bool				synthesizeMenu(PtWidget_t * wMenuRoot);
	UT_Bool				synthesizeMenu(void * wMenuRoot);
	UT_Bool				menuEvent(XAP_Menu_Id id);
	virtual UT_Bool		refreshMenu(AV_View * pView) = 0;

	XAP_QNXFrame * 	getFrame(void);

protected:

	//UT_Bool				_refreshMenu(AV_View * pView, GtkWidget * wMenuRoot);
	UT_Bool				_refreshMenu(AV_View * pView, void * wMenuRoot);
	UT_Bool				_isItemPresent(XAP_Menu_Id id) const;

	XAP_QNXApp *		m_pQNXApp;
	XAP_QNXFrame *		m_pQNXFrame;

	// Menu accelerator group, dynamically filled on synth()
//	GtkAccelGroup * 	m_accelGroup;
	
	// actual GTK menu widgets
	UT_Vector			m_vecMenuWidgets;
};

/*****************************************************************/

class EV_QNXMenuBar : public EV_QNXMenu
{
public:
	EV_QNXMenuBar(XAP_QNXApp * pQNXApp,
				   XAP_QNXFrame * pQNXFrame,
				   const char * szMenuLayoutName,
				   const char * szMenuLabelSetName);
	//virtual ~EV_QNXMenuBar(void);
	~EV_QNXMenuBar(void);

	UT_Bool				synthesizeMenuBar(void);
	virtual UT_Bool		refreshMenu(AV_View * pView);

protected:
	PtWidget_t *			m_wMenuBar;
	void * 		m_wHandleBox;
};

/*****************************************************************/

class EV_QNXMenuPopup : public EV_QNXMenu
{
public:
	EV_QNXMenuPopup(XAP_QNXApp * pQNXApp,
					 XAP_QNXFrame * pQNXFrame,
					 const char * szMenuLayoutName,
					 const char * szMenuLabelSetName);
	//virtual ~EV_QNXMenuPopup(void);
	~EV_QNXMenuPopup(void);

	UT_Bool				synthesizeMenuPopup();
	virtual UT_Bool		refreshMenu(AV_View * pView);
	virtual PtWidget_t *	getMenuHandle(void) const;


protected:
	PtWidget_t *			m_wMenuPopup;
};

#endif /* EV_QNXMENU_H */
