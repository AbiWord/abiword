 
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_Keyboard.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"

EV_Keyboard::EV_Keyboard(EV_EditEventMapper * pEEM)
{
	UT_ASSERT(pEEM);

	m_pEEM = pEEM;
}

UT_Bool EV_Keyboard::invokeKeyboardMethod(FV_View * pView,
										  EV_EditMethod * pEM,
										  UT_uint32 iPrefixCount,
										  UT_UCSChar * pData,
										  UT_uint32 dataLength)
{
	UT_ASSERT(pView);
	UT_ASSERT(pEM);

#ifdef UT_DEBUG
	UT_DEBUGMSG(("invokeKeyboardMethod: %s repeat %d length %d with [",
				 pEM->getName(),iPrefixCount,dataLength));
	if (pData && dataLength)
		for (UT_uint32 k=0; k<dataLength; k++)
			UT_DEBUGMSG(("%04x(%c) ",pData[k],pData[k]));
	UT_DEBUGMSG(("]\n"));
#endif	

	EV_EditMethodType t = pEM->getType();

	if (((t & EV_EMT_ALLOWMULTIPLIER) == 0) && (iPrefixCount != 1))
	{
		// they gave a prefix multiplier and this method doesn't permit it.
		// TODO should be beep or complain or just silently fail ?? 
		UT_DEBUGMSG(("    invoke aborted due to multiplier\n"));
		return UT_FALSE;
	}

	if (((t & EV_EMT_REQUIREDATA) != 0) && (!pData || !dataLength))
	{
		// This method requires character data and the caller did not provide any.
		UT_DEBUGMSG(("    invoke aborted due to lack of data\n"));
		return UT_FALSE;
	}

	EV_EditMethodCallData emcd(1,pData,dataLength);
	(*pEM->getFn())(pView,&emcd);

	return UT_TRUE;
	
}


