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
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_About.h"
#include "ap_MacDialog_All.h"
#include "ap_MacDialog_About.h"

#include "abisource.xpm"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete p; } while (0)


/*****************************************************************/
AP_Dialog * AP_MacDialog_About::static_constructor(AP_DialogFactory * pFactory,
															 AP_Dialog_Id id)
{
	AP_MacDialog_About * p = new AP_MacDialog_About(pFactory,id);
	return p;
}

AP_MacDialog_About::AP_MacDialog_About(AP_DialogFactory * pDlgFactory,
											 AP_Dialog_Id id)
	: AP_Dialog_About(pDlgFactory,id)
{
}

AP_MacDialog_About::~AP_MacDialog_About(void)
{
}

void AP_MacDialog_About::runModal(XAP_Frame * pFrame)
{

}

