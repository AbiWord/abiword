/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ap_Menu_Id.h"
#include "ap_Menu_Functions.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "xap_App.h"
#include "xap_Clipboard.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xav_View.h"


/*****************************************************************/
/*****************************************************************/

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_About)
{
        // Compute the menu label for the _help_about itema
	// .
	UT_ASSERT(pFrame);
	XAP_App * pApp = pFrame->getApp();
        UT_ASSERT(pApp);
        UT_ASSERT(pLabel);
        UT_ASSERT(id == AP_MENU_ID_HELP_ABOUT);

        const char * szFormat = pLabel->getMenuLabel();
        static char buf[128];

        const char * szAppName = pApp->getApplicationName();

        sprintf(buf,szFormat,szAppName);
        return buf;

        return NULL;
}




