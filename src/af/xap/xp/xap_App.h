 
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

#ifndef AP_AP_H
#define AP_AP_H

#include "ut_types.h"

class EV_EditMethodContainer;
class EV_Menu_ActionSet;

/*****************************************************************
******************************************************************
** This file defines the base class for the cross-platform 
** application.  This is used to hold all of the application-specific
** data.  Only one of these is created by the application.
******************************************************************
*****************************************************************/

class AP_Ap
{
public:
	AP_Ap(void);
	virtual ~AP_Ap(void);

	virtual UT_Bool					initialize(int argc, char ** argv);
	EV_EditMethodContainer *		getEditMethodContainer(void) const;
	const EV_Menu_ActionSet *		getMenuActionSet(void) const;

protected:
	EV_EditMethodContainer *	m_pEMC;				/* the set of all possible EditMethods in the app */
	EV_Menu_ActionSet *			m_pMenuActionSet;	/* the set of all possible menu actions in the app */

};

#endif /* AP_AP_H */
