/* Handle caret blinking.
 *
 * Author: Mike Nordell (tamlin@algonet.se)
 *         Pat Lam
 *         Dom Lachowicz
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

#include "fv_View.h"
#include "fv_Caret.h"
#include "ut_sleep.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

#define CURSOR_BLINK_TIME 500000

FV_Caret::FV_Caret(FV_View& view)
:	m_view(view),
	m_worker(*this),
	m_nDisableCount(1),
	m_bCursorBlink(true),
	m_bCursorIsOn(false)
{
	m_worker.setPriority(UT_Thread::PRI_LOW);
	m_worker.start();
}

FV_Caret::~FV_Caret()
{
	UT_MutexAcquirer lock(m_disableProtector);
	if (m_bCursorIsOn)
		m_view.eraseInsertionPoint();
	m_bCursorIsOn = false;
	m_worker.m_bDie = true;
}

void FV_Caret::enable()
{
	UT_DEBUGMSG(("FV_Caret::enable() , count = %d\n", m_nDisableCount));

	{
		UT_MutexAcquirer lock(m_disableProtector);
		if (m_nDisableCount == 0 && m_bCursorIsOn)
		{
			_blink();
		}

		if (m_nDisableCount == 0 || --m_nDisableCount)
		{
			_blink ();
			return;
		}
	}

	// Always start out with the caret in drawn state
	if (!m_bCursorIsOn)
	{
		_blink();
	}

	// Caret should always be "on" when leaving enable().
	UT_ASSERT(m_bCursorIsOn);	// post condition
}

void FV_Caret::disable()
{
	UT_DEBUGMSG(("FV_Caret::disable(), count = %d\n", m_nDisableCount));

	UT_MutexAcquirer lock(m_disableProtector);

	if (!m_nDisableCount++)
	{
		if (m_bCursorIsOn)
		{
			m_view.eraseInsertionPoint();
			m_bCursorIsOn = false;
		}
	}

	// Caret should never be "on" when leaving enable().
	UT_ASSERT(!m_bCursorIsOn);	// post condition
}

void FV_Caret::setBlink(bool bBlink)
{
	m_bCursorBlink = bBlink;
}

void FV_Caret::_blink()
{
	m_bCursorIsOn = !m_bCursorIsOn;
	const bool bTurnCursorOn = m_bCursorIsOn;

	// eventually we should move draw & erase here.
	if (bTurnCursorOn || !m_bCursorBlink)
	{
		if (!m_view._ensureThatInsertionPointIsOnScreen())
		{
			m_view._fixInsertionPointCoords();
			m_view.drawInsertionPoint();
		}
	}
	else
	{
		m_view.eraseInsertionPoint();
	}
}

void FV_Caret::Worker::run()
{
	while (!m_bDie)
	{
		m_owner.m_disableProtector.lock();
		if (m_owner.m_nDisableCount == 0)
			m_owner._blink();
		m_owner.m_disableProtector.unlock();
		UT_usleep(CURSOR_BLINK_TIME);
	}
}
