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

#ifndef EV_MACMENU_H
#define EV_MACMENU_H

#include "ut_types.h"
#include "xap_Types.h"
#include "ev_Menu.h"

class AV_View;
class AP_MacApp;
class XAP_MacFrame;


/*****************************************************************/

class EV_MacMenu : public EV_Menu
{
public:
	EV_MacMenu(AP_MacApp * pMacApp, XAP_MacFrame * pMacFrame,
				 const char * szMenuLayoutName,
				 const char * szMenuLabelSetName);
	~EV_MacMenu(void);

	UT_Bool onCommand(AV_View * pView, 
								WindowPtr hWnd, int wParam);
	
	UT_Bool synthesize(void);

	UT_Bool onInitMenu(AV_View * pView, WindowPtr hWnd, 
					Handle hMenuBar);


private:
	AP_MacApp *m_pMacApp;
	XAP_MacFrame *m_pMacFrame;

};

#endif /* EV_MACMENU_H */
