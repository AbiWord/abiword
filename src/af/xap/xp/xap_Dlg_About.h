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
#include "xav_View.h"

// NOTE: don't localize these through the strings mechanism
// NOTE: we need to be sure that legal text is built into the app

#define XAP_ABOUT_TITLE "About %s"
#define XAP_ABOUT_DESCRIPTION "%s is an Open Source application licensed under the GNU GPL.\nYou are free to redistribute this application."
#define XAP_ABOUT_COPYRIGHT "Copyright 1998, 1999 AbiSource, Inc."
#define XAP_ABOUT_GPL "%s is available for use under the the terms\nof the GNU General Public License"
#define XAP_ABOUT_VERSION "Version: %s"
#define XAP_ABOUT_BUILD "Build options: %s"
#define XAP_ABOUT_URL "For more information: http://www.abisource.com/"
#define XAP_ABOUT_GPL_LONG "AbiWord and AbiSource are trademarks of AbiSource, Inc.\r\n\r\n%s is free software; you can redistribute it and/or \
modify it under the terms of the GNU General Public License \
as published by the Free Software Foundation; either version 2 \
of the License, or (at your option) any later version.\r\n\r\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details."

#define XAP_ABOUT_GPL_LONG_LINE_BROKEN "AbiWord and AbiSource are trademarks of\nAbiSource, Inc.\n\n%s is free software; you can redistribute it\n \
and/or modify it under the terms of the GNU General\n \
Public License as published by the Free Software\n \
Foundation; either version 2 of the License, or (at your\n \
option) any later version.\r\n\r\n \
This program is distributed in the hope that it will be useful,\n \
but WITHOUT ANY WARRANTY; without even the\n \
implied warranty of MERCHANTABILITY or FITNESS\n \
FOR A PARTICULAR PURPOSE.  See the GNU General\n \
Public License for more details."

#define XAP_ABOUT_GPL_LONG_LF "AbiWord and AbiSource are trademarks of AbiSource, Inc.\n\n%s is free software; you can redistribute it and/or \
modify it under the terms of the GNU General Public License \
as published by the Free Software Foundation; either version 2 \
of the License, or (at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details."

class XAP_Dialog_About : public XAP_Dialog_NonPersistent
{
 public:
	XAP_Dialog_About(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_About(void);

	virtual void				runModal(XAP_Frame * pFrame) = 0;

 protected:

};

#endif // XAP_DIALOG_ABOUT_H
