 
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
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef UT_TIMER_H
#define UT_TIMER_H

#include "ut_types.h"
#include "ut_vector.h"

class UT_Timer;

typedef void (*UT_TimerCallback)(UT_Timer* pTimer);

class UT_Timer
{
public:
	UT_Timer();
	
	void setCallback(UT_TimerCallback p);
	UT_TimerCallback getCallback();
	
	void setInstanceData(void*);
	void* getInstanceData();
	
	virtual UT_sint32 set(UT_uint32 iMilliseconds) = 0;
	void fire();
	
	void setIdentifier(UT_uint32);
	UT_uint32 getIdentifier();
	
	static UT_Timer* findTimer(UT_uint32 iIdentifier);
	
protected:
	void* m_pInstanceData;
	UT_TimerCallback m_pCallback;
	UT_uint32 m_iIdentifier;

	static UT_Vector static_vecTimers;
};

#endif /* UT_TIMER_H */
