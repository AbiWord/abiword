 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef EV_WIN32MENU_H
#define EV_WIN32MENU_H

#include <windows.h>
#include "ut_types.h"
#include "ev_Menu.h"
#include "ap_Menu_Id.h"
class FV_View;
class AP_Win32Ap;
class AP_Win32Frame;

/*****************************************************************/

class EV_Win32Menu : public EV_Menu
{
public:
	EV_Win32Menu(AP_Win32Ap * pWin32Ap, AP_Win32Frame * pWin32Frame);
	~EV_Win32Menu(void);

	UT_Bool				synthesize(void);
	UT_Bool				onCommand(FV_View * pView, HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

protected:
	AP_Win32Ap *		m_pWin32Ap;
	AP_Win32Frame *		m_pWin32Frame;
};

#endif /* EV_WIN32MENU_H */
