 
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_UnixMouse.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"


EV_UnixMouse::EV_UnixMouse(EV_EditEventMapper * pEEM) : EV_Mouse(pEEM)
{

}

void EV_UnixMouse::mouseClick(FV_View* pView, GdkEventButton* e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState state = 0;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	EV_EditMouseOp mop = 0;

	//      the following line was removed.  using '3' seems to work properly. -Eric
	//	else if (e->button == GDK_3BUTTON_PRESS)

	if (e->button == 1)
		emb = EV_EMB_BUTTON1;
	else if (e->button == 2)
		emb = EV_EMB_BUTTON2;
	else if (e->button == 3)
		emb = EV_EMB_BUTTON3;
		
	if (e->state & GDK_SHIFT_MASK)
		state |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		state |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		state |= EV_EMS_ALT;

	if (e->type == GDK_BUTTON_PRESS)
		mop = EV_EMO_SINGLECLICK;
	else if (e->type == GDK_BUTTON_RELEASE)
		mop = EV_EMO_RELEASE;
	
	result = m_pEEM->Mouse(mop|emb|state, &pEM,&iPrefix);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,iPrefix,e->x,e->y);
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}

void EV_UnixMouse::mouseMotion(FV_View* pView, GdkEventMotion *e)
{
	EV_EditMethod * pEM;
	EV_EditModifierState ems;
	UT_uint32 iPrefix;
	EV_EditEventMapperResult result;
	EV_EditMouseButton emb = 0;
	
	ems = 0;
	
	if (e->state & GDK_SHIFT_MASK)
		ems |= EV_EMS_SHIFT;
	if (e->state & GDK_CONTROL_MASK)
		ems |= EV_EMS_CONTROL;
	if (e->state & GDK_MOD1_MASK)
		ems |= EV_EMS_ALT;

	if (e->state & GDK_BUTTON1_MASK)
		emb = EV_EMB_BUTTON1;
	else if (e->state & GDK_BUTTON2_MASK)
		emb = EV_EMB_BUTTON2;
	else if (e->state & GDK_BUTTON3_MASK)
		emb = EV_EMB_BUTTON3;

	// report movements under the mouse button that we did the capture on

	UT_DEBUGMSG(("onButtonMove: 0x%08lx [b=%d m=%d]\n",EV_EMO_DRAG|ems, emb, ems));
	
	result = m_pEEM->Mouse(EV_EMO_DRAG|emb|ems, &pEM,&iPrefix);
	
	switch (result)
	{
	case EV_EEMR_COMPLETE:
		UT_ASSERT(pEM);
		invokeMouseMethod(pView,pEM,iPrefix,e->x,e->y);
		return;
	case EV_EEMR_INCOMPLETE:
		// I'm not sure this makes any sense, but we allow it.
		return;
	case EV_EEMR_BOGUS_START:
	case EV_EEMR_BOGUS_CONT:
		// TODO What to do ?? Should we beep at them or just be quiet ??
		return;
	default:
		UT_ASSERT(0);
		return;
	}
}
