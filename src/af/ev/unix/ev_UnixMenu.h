 
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

#ifndef EV_UNIXMENU_H
#define EV_UNIXMENU_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ev_Menu.h"
#include "ap_Menu_Id.h"
class FV_View;
class AP_UnixAp;
class AP_UnixFrame;

/*****************************************************************/

class EV_UnixMenu : public EV_Menu
{
public:
	EV_UnixMenu(AP_UnixAp * pUnixAp, AP_UnixFrame * pUnixFrame);
	~EV_UnixMenu(void);

	UT_Bool				synthesize(void);
	UT_Bool				menuEvent(AP_Menu_Id id);

protected:
	AP_UnixAp *			m_pUnixAp;
	AP_UnixFrame *		m_pUnixFrame;

	GtkWidget *			m_wMenuBar;
	UT_Vector			m_vecMenuWidgets;
};

#endif /* EV_UNIXMENU_H */
