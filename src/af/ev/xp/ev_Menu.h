 
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


#ifndef EV_MENU_H
#define EV_MENU_H

#include "ut_types.h"

class EV_EditMethodContainer;
class EV_EditMethod;
class FV_View;

class EV_Menu
{
public:
	EV_Menu(EV_EditMethodContainer * pEMC);
	UT_Bool invokeMenuMethod(FV_View * pView,
								 EV_EditMethod * pEM,
								 UT_uint32 iPrefixCount,
								 UT_UCSChar * pData,
								 UT_uint32 dataLength);
	
protected:
	EV_EditMethodContainer *	m_pEMC;
};

#endif /* EV_MENU_H */
