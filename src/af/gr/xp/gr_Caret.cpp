/* An autonomous caret class.
 *
 * Author: Patrick Lam
 *         Dom Lachowicz
 * Inspired by Mike Nordell (tamlin@algonet.se)
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

#include "gr_Caret.h"
#include "gr_Graphics.h"
#include "ut_sleep.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

#define CURSOR_BLINK_TIME 600 /* milliseconds */
#define CURSOR_DELAY_TIME 10

// Description of m_enabler:
// The problem is that a complicated draw operation will be somewhat
// inefficient & ugly, because the caret gets drawn/undrawn every time
// one of the primitive draw operations takes place.  The solution
// there is for the caret to only be reinstated after the whole draw
// operation is done.  A good heuristic for that is to only draw the
// caret after 10ms of no drawing activity.  This can be done by
// scheduling the reinstating _blink for 10ms in the future; if
// there's another draw that happens to take place before 10ms, then
// your scheduled _blink gets cancelled.

GR_Caret::GR_Caret(GR_Graphics * pG)
	:   m_pClr(NULL),
		m_pG(pG),
		m_nDisableCount(1),
		m_bCursorBlink(true),
		m_bCursorIsOn(false),
		m_bPositionSet(false),
		m_bRecursiveDraw(false)
{
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	m_worker = static_cast<UT_Timer *>(UT_WorkerFactory::static_constructor
		(s_work, this, UT_WorkerFactory::TIMER, outMode, pG));
	UT_ASSERT(outMode == UT_WorkerFactory::TIMER);
	m_worker->set(CURSOR_BLINK_TIME);

	m_enabler = static_cast<UT_Timer *>(UT_WorkerFactory::static_constructor
		(s_enable, this, UT_WorkerFactory::TIMER, outMode, pG));
	UT_ASSERT(outMode == UT_WorkerFactory::TIMER);
	m_enabler->set(CURSOR_DELAY_TIME);
}

void GR_Caret::s_work(UT_Worker * _w)
{
	GR_Caret * c = static_cast<GR_Caret *>(_w->getInstanceData());

 	xxx_UT_DEBUGMSG(("blinking cursor %d\n", c->m_nDisableCount));

	if (c->m_nDisableCount == 0)
		c->_blink(false);
}

/** One-time enabler. */
void GR_Caret::s_enable(UT_Worker * _w)
{
	GR_Caret * c = static_cast<GR_Caret *>(_w->getInstanceData());

 	xxx_UT_DEBUGMSG(("enabling cursor %d\n", c->m_nDisableCount));

	c->_blink();
	c->m_worker->start();
	c->m_enabler->stop();
}

GR_Caret::~GR_Caret()
{
	m_worker->stop();
	m_enabler->stop();
}

void GR_Caret::setCoords(UT_sint32 x, UT_sint32 y, UT_uint32 h,
						 UT_sint32 x2, UT_sint32 y2, UT_uint32 h2,
						 bool bPointDirection, UT_RGBColor * pClr)
{
	// if visible, then hide while we change positions.
 	if (m_bCursorIsOn)
 		_blink();

	m_xPoint = x; m_yPoint = y; m_iPointHeight = h;
	m_xPoint2 = x2; m_yPoint2 = y2; m_iPointHeight2 = h2;
	m_bPointDirection = bPointDirection; m_pClr = pClr;
	m_bPositionSet = true;

	// now show the cursor, if it's enabled, and restart the timer.
	// if we don't do this, the cursor is invisible during cursor motion.
	if (m_nDisableCount == 0)
	{
 		_blink(false);
 		_blink();
	}
}


void GR_Caret::enable()
{
	if (m_bRecursiveDraw)
		return;

  	xxx_UT_DEBUGMSG(("GR_Caret::enable(), this=%p, count = %d\n", this, m_nDisableCount));
	if (m_nDisableCount == 0)
	{
		// If the caret is already enabled, we re-draw the caret
		// and don't touch m_nDisableCount.
		if (m_bCursorIsOn)
			_blink();
		_blink();
		return;
	}

	--m_nDisableCount;
	// Check to see if we still have pending disables.
	if (m_nDisableCount)
		return;

	// stop pending enables; in 10 ms, really enable blinking.
	m_enabler->stop();
	m_enabler->start();
}

void GR_Caret::disable(bool bNoMulti)
{
	if (m_bRecursiveDraw)
		return;

   	xxx_UT_DEBUGMSG(("GR_Caret::disable(), this=%p, count = %d\n", this, m_nDisableCount));
	if (bNoMulti && (m_nDisableCount > 0))
		return;

	m_nDisableCount++;
	if ((m_nDisableCount == 1) && m_bCursorIsOn)
		_blink(true, true);

	m_worker->stop();
	m_enabler->stop();
	// Caret should never be "on" when leaving disable().
  	UT_ASSERT(m_bRecursiveDraw || !m_bPositionSet || !m_bCursorBlink ||
			  (m_nDisableCount != 0) || !m_bCursorIsOn);
}

/** Determines whether Abi is going to blink the cursor or not.
 * If not, then _blink() won't actually clear the cursor; it'll only draw. */
void GR_Caret::setBlink(bool bBlink)
{
	m_bCursorBlink = bBlink;
}

void GR_Caret::_blink(bool bExplicit, bool bForceClear)
{
	if (m_bRecursiveDraw || !m_bPositionSet)
		return;

	// After any blink, we want there to be BLINK_TIME until next autoblink.
	if (!bExplicit)
	{ 
		m_worker->stop(); m_worker->start();
	}

	UT_ASSERT(m_bCursorBlink);
	if (m_bCursorBlink || (bForceClear && !m_bCursorIsOn))
	{
		m_bRecursiveDraw = true;
		
		xxx_UT_DEBUGMSG(("actually blinking at %d %d h=%d; cursorison will be %s\n", m_xPoint, m_yPoint, m_iPointHeight, !m_bCursorIsOn ? "true" : "false"));
		m_bCursorIsOn = !m_bCursorIsOn;

		UT_RGBColor oldColor; m_pG->getColor(oldColor);
  		if (m_pClr)
			m_pG->setColor(*m_pClr);
		else
			m_pG->setColor(UT_RGBColor(255,255,255));

		m_pG->xorLine(m_xPoint-1, m_yPoint+1, m_xPoint-1, 
					  m_yPoint + m_iPointHeight+1);
		m_pG->xorLine(m_xPoint, m_yPoint+1, m_xPoint, 
					  m_yPoint + m_iPointHeight+1);

		if((m_xPoint != m_xPoint2) || (m_yPoint != m_yPoint2))
		{
			// #TF the caret will have a small flag at the top 
			// indicating the direction of writing
			if(m_bPointDirection)
			{
				m_pG->xorLine(m_xPoint-3, m_yPoint+1, m_xPoint-1, m_yPoint+1);
				m_pG->xorLine(m_xPoint-2, m_yPoint+2, m_xPoint-1, m_yPoint+2);
			}
			else
			{
				m_pG->xorLine(m_xPoint+1, m_yPoint+1, m_xPoint+3, m_yPoint+1);
				m_pG->xorLine(m_xPoint+1, m_yPoint+2, m_xPoint+2, m_yPoint+2);
			}

			// This is the second caret on ltr-rtl boundary
			m_pG->xorLine(m_xPoint2-1, m_yPoint2+1, 
						  m_xPoint2-1, m_yPoint2 + m_iPointHeight + 1);
			m_pG->xorLine(m_xPoint2, m_yPoint2+1, 
						  m_xPoint2, m_yPoint2 + m_iPointHeight + 1);

			// This is the line that links the two carets
			m_pG->xorLine(m_xPoint, m_yPoint + m_iPointHeight + 1, 
						  m_xPoint2, m_yPoint2 + m_iPointHeight + 1);

			if(m_bPointDirection)
			{
				m_pG->xorLine(m_xPoint2+1, m_yPoint2+1, 
							  m_xPoint2+3, m_yPoint2+1);
				m_pG->xorLine(m_xPoint2+1, m_yPoint2+2, 
							  m_xPoint2+2, m_yPoint2+2);
			}
			else
			{
				m_pG->xorLine(m_xPoint2-3, m_yPoint2+1, 
							  m_xPoint2-1, m_yPoint2+1);
				m_pG->xorLine(m_xPoint2-2, m_yPoint2+2, 
							  m_xPoint2-1, m_yPoint2+2);
			}
		}
		m_pG->setColor(oldColor);
		m_bRecursiveDraw = false;
	}
}
