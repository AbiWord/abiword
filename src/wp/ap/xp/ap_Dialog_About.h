/* AbiSource Application Framework
 * Copyright (C) 1998, 1999 AbiSource, Inc.
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

#ifndef AP_DIALOG_ABOUT_H
#define AP_DIALOG_ABOUT_H

#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "pt_Types.h"

#define ABOUT_COPYRIGHT   "Copyright 1998, 1999 AbiSource, Inc."
#define ABOUT_GPL         "AbiWord is available for use under the terms\nof the GNU General Public License"
#define ABOUT_URL         "For source and support: http://www.abisource.com/"
#define ABOUT_DESCRIPTION "AbiWord is Open Source Software, licensed under the GPL.\nThere are no other licensing retrictions placed upon you\nYou may freely distribute this product."

class AP_Dialog_About : public AP_Dialog_AppPersistent
{
 public:
	AP_Dialog_About(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_About(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

 protected:
	char* strCopyright;
	char* strGPL;
	char* strVersion;
	char* strBuildInfo;
	char* strURL;
};

#endif // AP_DIALOG_ABOUT_H
