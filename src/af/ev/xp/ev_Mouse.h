 
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


#ifndef EV_MOUSE_H
#define EV_MOUSE_H

#include "ut_types.h"
class EV_EditEventMapper;
class EV_EditMethod;
class FV_View;

class EV_Mouse
{
public:
	EV_Mouse(EV_EditEventMapper * pEEM);
	UT_Bool invokeMouseMethod(FV_View * pView,
							  EV_EditMethod * pEM,
							  UT_uint32 iPrefixCount,
							  UT_uint32 xPos,
							  UT_uint32 yPos);

protected:
	EV_EditEventMapper *	m_pEEM;
};

#endif /* EV_MOUSE_H */
