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

#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_About.h"
#include "ap_Win32Dialog_All.h"
#include "ap_Win32Dialog_About.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete p; } while (0)


/*****************************************************************/
AP_Dialog * AP_Win32Dialog_About::static_constructor(AP_DialogFactory * pFactory,
															 AP_Dialog_Id id)
{
	AP_Win32Dialog_About * p = new AP_Win32Dialog_About(pFactory,id);
	return p;
}

AP_Win32Dialog_About::AP_Win32Dialog_About(AP_DialogFactory * pDlgFactory,
											 AP_Dialog_Id id)
	: AP_Dialog_About(pDlgFactory,id)
{
}

AP_Win32Dialog_About::~AP_Win32Dialog_About(void)
{
}

void AP_Win32Dialog_About::runModal(XAP_Frame * pFrame)
{
}
