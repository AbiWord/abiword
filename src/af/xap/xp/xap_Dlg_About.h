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

#define XAP_ABOUT_COPYRIGHT "Copyright 1998, 1999 AbiSource, Inc."
#define XAP_ABOUT_DESCRIPTION "%s is an Open Source application licensed under the GPL.\nYou are free to redistribute this product"
#define XAP_ABOUT_GPL "%s is available for use under the therms\nof the GNU General Public License"
#define XAP_ABOUT_URL "For source and support: http://www.abisource.com/"

class AP_Dialog_About : public AP_Dialog_NonPersistent
{
 public:
	AP_Dialog_About(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_About(void);

	virtual void				runModal(XAP_Frame * pFrame) = 0;

 protected:

};

#endif // AP_DIALOG_ABOUT_H
