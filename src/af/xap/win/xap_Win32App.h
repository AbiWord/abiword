/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef AP_WIN32AP_H
#define AP_WIN32AP_H

#include <windows.h>
#include "ap_Ap.h"

/*****************************************************************
******************************************************************
** This file defines the Win32-platform-specific class for the
** cross-platform application.  This is used to hold all of the
** platform-specific, application-specific data.  Only one of these
** is created by the application.
******************************************************************
*****************************************************************/

class AP_Win32Ap : public AP_Ap
{
public:
	AP_Win32Ap(HINSTANCE hInstance);
	virtual ~AP_Win32Ap(void);

	virtual UT_Bool			initialize(int * pArgc, char *** pArgv);

	virtual HINSTANCE		getInstance() const;

protected:
	HINSTANCE	m_hInstance;

	/* TODO put anything we need here.  
	*/
};

#endif /* AP_WIN32AP_H */
