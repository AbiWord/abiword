/* An autonomous caret class.
 *
 * Author: Patrick Lam
 *         Dom Lachowicz
 *         Tomas Frydrych
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
#include "ap_Frame.h"
#include "ap_FrameData.h"
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

GR_Caret::GR_Caret(GR_Graphics * pG, XAP_Frame * pFrame)
	:   m_pClr(NULL),
	    m_pG(pG),
	    m_nDisableCount(1),
	    m_bCursorBlink(true),
	    m_bCursorIsOn(false),
	    m_bPositionSet(false),
		m_bRecursiveDraw(false),
		m_bSplitCaret(false),
		m_bCaret1OnScreen(false),
		m_bCaret2OnScreen(false),
		m_clrInsert(0,0,0),
		m_clrOverwrite(255,0,0)
{

	m_pListener = new GR_Caret_Listener(pFrame);
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

GR_Caret::~GR_Caret()
{
	m_worker->stop();
	m_enabler->stop();

	DELETEP(m_worker);
	DELETEP(m_enabler);

	xxx_UT_DEBUGMSG(("DOM: deleting caret %p\n", this));
	DELETEP(m_pListener);
}

void GR_Caret::s_work(UT_Worker * _w)
{
	GR_Caret * c = static_cast<GR_Caret *>(_w->getInstanceData());

	if (c->m_nDisableCount == 0)
		c->_blink(false);
}

/** One-time enabler. */
void GR_Caret::s_enable(UT_Worker * _w)
{
	GR_Caret * c = static_cast<GR_Caret *>(_w->getInstanceData());

	c->m_worker->stop();
	if (!c->m_bCursorIsOn)
	{
		c->_blink(true);
	}
	else
	{
		c->_blink(true);
		c->_blink(true);
	}
	c->m_worker->start();
	c->m_enabler->stop();
}

void GR_Caret::setCoords(UT_sint32 x, UT_sint32 y, UT_uint32 h,
			 UT_sint32 x2, UT_sint32 y2, UT_uint32 h2,
			 bool bPointDirection, UT_RGBColor * pClr)
{
	// if visible, then hide while we change positions.
	_erase();

	m_xPoint = x; m_yPoint = y; m_iPointHeight = h;
	m_xPoint2 = x2; m_yPoint2 = y2; m_iPointHeight2 = h2;
	m_bPointDirection = bPointDirection; m_pClr = pClr;
	m_bPositionSet = true;

	// This is a partial on-screen logic; we currently only check for
	// negative coordinances, i.e., caret above and/or to the left of
	// the editing window. To check for the other side would be more
	// complicated, we would need to have a listener watching for
	// window resizing. Tomas Jan 18, 2003
	if(x < 0 || y < 0)
		m_bCaret1OnScreen = false;
	else
		m_bCaret1OnScreen = true;
	
	if(x2 < 0 || y2 < 0)
		m_bCaret2OnScreen = false;
	else
		m_bCaret2OnScreen = true;

	// now show the caret, if it's enabled, and restart the timer.
	// if we don't do this, the caret is invisible during caret motion.
	// For some reason, we seem to do OK now for that.  I don't get it.
	//if (m_nDisableCount == 0)
	//{
	//	m_worker->stop(); m_worker->start();
	//}
}

void GR_Caret::enable()
{
	if (m_bRecursiveDraw)
		return;

	if (m_nDisableCount == 0)
	{
		// If the caret is already enabled, just return
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

	if (bNoMulti && (m_nDisableCount > 0))
		return;

	m_nDisableCount++;
	if ((m_nDisableCount == 1) && m_bCursorIsOn)
		_erase();

	m_worker->stop();
	m_enabler->stop();
	// Caret should never be "on" when leaving disable().
  	UT_ASSERT(m_bRecursiveDraw || !m_bPositionSet || !m_bCursorBlink ||
			  (m_nDisableCount != 0) || !m_bCursorIsOn);
}

/** Determines whether Abi is going to blink the caret or not.
 * If not, then _blink() won't actually clear the caret; it'll only draw. */
void GR_Caret::setBlink(bool bBlink)
{
	m_bCursorBlink = bBlink;
}

void GR_Caret::_erase()
{
	if (m_bCursorIsOn)
		_blink(true);
	UT_ASSERT(!m_bCursorIsOn);
}

void GR_Caret::_blink(bool bExplicit)
{
	if (m_bRecursiveDraw || !m_bPositionSet)
		return;

	// After any autoblink, we want there to be BLINK_TIME 
	// until next autoblink.
	if (!bExplicit)
	{ 
		m_worker->stop(); m_worker->start();
	}

	// Blink if: (a) _blink explicitly called (not autoblink); or
	//           (b) autoblink and caret blink enabled; or
	//           (c) autoblink, caret blink disabled, caret is off
	if (bExplicit || m_bCursorBlink || !m_bCursorIsOn)
	{
		m_bRecursiveDraw = true;
		
		UT_RGBColor oldColor; m_pG->getColor(oldColor);

		if (m_bCursorIsOn)
		{
			m_pG->restoreRectangle(0);

			if(m_bSplitCaret)
			{
				m_pG->restoreRectangle(1);
				m_pG->restoreRectangle(2);
				m_bSplitCaret = false;
			}
		}
		else
		{
			// if neither caret is on screen, quit
			if(!m_bCaret1OnScreen && !m_bCaret2OnScreen)
			{
				m_bCursorIsOn = false;
				m_bRecursiveDraw = false;
				return;
			}
			
			UT_Rect r0(m_xPoint-3, m_yPoint+1, 7, m_iPointHeight);
			m_pG->saveRectangle(r0,0);

			if((m_xPoint != m_xPoint2) || (m_yPoint != m_yPoint2))
			{
				m_bSplitCaret = true;
				// have to save the rectangle for the joining line
				// before we draw the carets
				UT_uint32 xmin = UT_MIN(m_xPoint, m_xPoint2);
				UT_uint32 xmax = UT_MAX(m_xPoint, m_xPoint2);
				UT_uint32 ymin = UT_MIN(m_yPoint, m_yPoint2);
				UT_uint32 ymax = UT_MAX(m_yPoint, m_yPoint2);
			
				UT_Rect r2(xmin-1, ymin + m_iPointHeight, xmax - xmin + 2, ymax - ymin + 1);
				m_pG->saveRectangle(r2,2);
			}
			else
				m_bSplitCaret = false;

			//static const UT_RGBColor black (0,0,0);
			if(static_cast<GR_Caret_Listener*>(getListener())->getInsertMode())
				m_pG->setColor(m_clrInsert);
			else
				m_pG->setColor(m_clrOverwrite);

			if(m_bCaret1OnScreen)
			{
				m_pG->drawLine(m_xPoint-1, m_yPoint+1, m_xPoint-1, 
							   m_yPoint + m_iPointHeight+1);
				m_pG->drawLine(m_xPoint, m_yPoint+1, m_xPoint, 
							   m_yPoint + m_iPointHeight+1);
			}
			
			if(m_bSplitCaret)
			{
				// #TF the caret will have a small flag at the top 
				// indicating the direction of writing
				if(m_bCaret1OnScreen)
				{
					if(m_bPointDirection)
					{
						m_pG->drawLine(m_xPoint-3, m_yPoint+1, m_xPoint-1, m_yPoint+1);
						m_pG->drawLine(m_xPoint-2, m_yPoint+2, m_xPoint-1, m_yPoint+2);
					}
					else
					{
						m_pG->drawLine(m_xPoint+1, m_yPoint+1, m_xPoint+3, m_yPoint+1);
						m_pG->drawLine(m_xPoint+1, m_yPoint+2, m_xPoint+2, m_yPoint+2);
					}
				}
				
				// This is the second caret on ltr-rtl boundary

				if(m_bCaret2OnScreen)
				{
					UT_Rect r1(m_xPoint2-3, m_yPoint2+1, 7, m_iPointHeight);
					m_pG->saveRectangle(r1,1);
				
			
					m_pG->drawLine(m_xPoint2-1, m_yPoint2+1, 
								   m_xPoint2-1, m_yPoint2 + m_iPointHeight + 1);
					m_pG->drawLine(m_xPoint2, m_yPoint2+1, 
								   m_xPoint2, m_yPoint2 + m_iPointHeight + 1);

					// This is the line that links the two carets
					m_pG->drawLine(m_xPoint, m_yPoint + m_iPointHeight, 
								   m_xPoint2, m_yPoint2 + m_iPointHeight);

					if(m_bPointDirection)
					{
						m_pG->drawLine(m_xPoint2+1, m_yPoint2+1, 
									   m_xPoint2+3, m_yPoint2+1);
						m_pG->drawLine(m_xPoint2+1, m_yPoint2+2, 
									   m_xPoint2+2, m_yPoint2+2);
					}
					else
					{
						m_pG->drawLine(m_xPoint2-3, m_yPoint2+1, 
									   m_xPoint2-1, m_yPoint2+1);
						m_pG->drawLine(m_xPoint2-2, m_yPoint2+2, 
									   m_xPoint2-1, m_yPoint2+2);
					}
				}
				
			}
			
		}

		m_bCursorIsOn = !m_bCursorIsOn;

 		m_pG->setColor(oldColor);

		m_bRecursiveDraw = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
GR_Caret_Listener::GR_Caret_Listener(XAP_Frame * pFrame):
	m_pFrame(pFrame),
	m_bInsertMode(true)
{
	UT_ASSERT( m_pFrame );
}



bool GR_Caret_Listener::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
    if (m_pFrame && (mask & (AV_CHG_INSERTMODE)))
    {
		AP_FrameData * pData = static_cast<AP_FrameData *>(m_pFrame->getFrameData());
		if (pData) {
			m_bInsertMode = pData->m_bInsertMode;
			xxx_UT_DEBUGMSG(("GR_Caret_Listener: InsertMode %d\n", m_bInsertMode));
			return true;
		}
    }
	return false;
}
