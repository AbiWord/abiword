/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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


#include "ut_MacTimer.h"
#include "ut_assert.h"

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback,
									   void* pData,
									   GR_Graphics *g)
{
	UT_ASSERT(pCallback);

	UT_Timer * p = new UT_MacTimer();

	if (p)
	{
		p->setCallback(pCallback);
		p->setInstanceData(pData);
	}

	return p;
}

/*****************************************************************/

UT_MacTimer::~UT_MacTimer()
{
}

UT_sint32 UT_MacTimer::set(UT_uint32 iMilliseconds)
{
	return 0;
}

