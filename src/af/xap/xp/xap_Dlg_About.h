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

#ifndef XAP_DIALOG_ABOUT_H
#define XAP_DIALOG_ABOUT_H

#include "xap_Dialog.h"
#include "fv_View.h"
#include "xav_View.h"
#include "pt_Types.h"

#define XAP_ABOUT_TITLE "About %s"
#define XAP_ABOUT_DESCRIPTION "%s is an Open Source application licensed under the GNU GPL.\nYou are free to redistribute this application."
#define XAP_ABOUT_COPYRIGHT "Copyright 1998, 1999 AbiSource, Inc."
#define XAP_ABOUT_GPL "%s is available for use under the the terms\nof the GNU General Public License"
#define XAP_ABOUT_VERSION "Version: %s"
#define XAP_ABOUT_BUILD "Build options: %s"
#define XAP_ABOUT_URL "For more information: http://www.abisource.com/"

class XAP_Dialog_About : public AP_Dialog_NonPersistent
{
 public:
	XAP_Dialog_About(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~XAP_Dialog_About(void);

	virtual void				runModal(XAP_Frame * pFrame) = 0;

 protected:

};

#endif // XAP_DIALOG_ABOUT_H
