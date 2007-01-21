/* An autonomous caret class.
 *
 * Authors: Patrick Lam
 *          Dom Lachowicz
 *          Tomas Frydrych
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
#include "gr_Painter.h"
#include "ut_debugmsg.h"
static const UT_uint32 CURSOR_DELAY_TIME = 10; // milliseconds

#ifdef XP_UNIX_TARGET_GTK
#include <gtk/gtk.h>
#elif defined(WIN32)
#include <windows.h>
#endif

UT_uint32 GR_Caret::getCursorBlinkTime () const
{
#ifdef XP_UNIX_TARGET_GTK
	UT_uint32 blink;
	GtkSettings * settings = gtk_settings_get_default ();

	g_object_get (G_OBJECT(settings), "gtk-cursor-blink-time", &blink, NULL);

	return (blink/2);
#elif defined(WIN32)
	return GetCaretBlinkTime ();
#else
	return 600; // milliseconds
#endif
}

bool GR_Caret::getCanCursorBlink () const
{
	return m_bCursorBlink;
}

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
	:  	m_xPoint(0), // init the x and y point to some value, since we don't have a sane value here
		m_yPoint(0),
		m_xPoint2(0),
		m_yPoint2(0),
		m_pClr(NULL),
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
		m_clrOverwrite(255,0,0),
		m_insertMode (true),
		m_bRemote(false),
		m_clrRemote(0,0,0),
		m_sDocUUID(""),
		m_iCaretNumber(0)
{
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	m_worker = static_cast<UT_Timer *>(UT_WorkerFactory::static_constructor
		(s_work, this, UT_WorkerFactory::TIMER, outMode));
	m_worker->set(getCursorBlinkTime ());

	m_enabler = static_cast<UT_Timer *>(UT_WorkerFactory::static_constructor
		(s_enable, this, UT_WorkerFactory::TIMER, outMode));
	m_enabler->set(CURSOR_DELAY_TIME);
	
	setBlink (false);
}


GR_Caret::GR_Caret(GR_Graphics * pG, UT_UTF8String & sDocUUID)
	:  	m_xPoint(0), // init the x and y point to some value, since we don't have a sane value here
		m_yPoint(0),
		m_xPoint2(0),
		m_yPoint2(0),
		m_pClr(NULL),
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
		m_clrOverwrite(255,0,0),
		m_insertMode (true),
		m_bRemote(true),
		m_clrRemote(0,0,0),
		m_sDocUUID(sDocUUID),
		m_iCaretNumber(0)
{
	UT_WorkerFactory::ConstructMode outMode = UT_WorkerFactory::NONE;
	m_worker = static_cast<UT_Timer *>(UT_WorkerFactory::static_constructor
		(s_work, this, UT_WorkerFactory::TIMER, outMode));
	m_worker->set(getCursorBlinkTime ());

	m_enabler = static_cast<UT_Timer *>(UT_WorkerFactory::static_constructor
		(s_enable, this, UT_WorkerFactory::TIMER, outMode));
	m_enabler->set(CURSOR_DELAY_TIME);
	m_iCaretNumber = static_cast<UT_sint32>(pG->m_vecCarets.getItemCount()) + 1;
	setBlink (false);
}

GR_Caret::~GR_Caret()
{
	m_worker->stop();
	m_enabler->stop();

	DELETEP(m_worker);
	DELETEP(m_enabler);
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
	c->_blink(true);
	if (!c->m_bCursorIsOn)
		c->_blink(true); // blink again
	else
	{
		c->_blink(true);
		c->_blink(true);
	}
	c->m_worker->start();
	c->m_enabler->stop();
}

void GR_Caret::setRemoteColor(UT_RGBColor clrRemote)
{
        m_clrRemote = clrRemote;
}

UT_UTF8String GR_Caret::getUUID(void)
{
        return m_sDocUUID;
}


void GR_Caret::setWindowSize(UT_uint32 width, UT_uint32 height)
{
	m_iWindowWidth = width; m_iWindowHeight = height;

	if(m_xPoint < m_pG->tlu(3)+1 || m_yPoint < 0 || m_xPoint > static_cast<UT_sint32>(m_iWindowWidth) || m_yPoint > static_cast<UT_sint32>(m_iWindowHeight))
		m_bCaret1OnScreen = false;
	else
		m_bCaret1OnScreen = true;
	
	if(m_xPoint2 < m_pG->tlu(3)+1 || m_yPoint2 < 0 || m_xPoint2 > static_cast<UT_sint32>(m_iWindowWidth) || m_yPoint2 > static_cast<UT_sint32>(m_iWindowHeight))
		m_bCaret2OnScreen = false;
	else
		m_bCaret2OnScreen = true;
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
	if(m_bRemote)
	  {
	    UT_DEBUGMSG(("Remote caret %x set to x %d \n",this,x)); 
	  }
	if(x < m_pG->tlu(3)+1 || y <= 0 || x > static_cast<UT_sint32>(m_iWindowWidth) || y > static_cast<UT_sint32>(m_iWindowHeight))
		m_bCaret1OnScreen = false;
	else
		m_bCaret1OnScreen = true;
	
	if(x2 < m_pG->tlu(3)+1 || y2 <= 0 || x2 > static_cast<UT_sint32>(m_iWindowWidth) || y2 > static_cast<UT_sint32>(m_iWindowHeight))
		m_bCaret2OnScreen = false;
	else
		m_bCaret2OnScreen = true;
}

void GR_Caret::enable()
{
	if (m_bRecursiveDraw)
		return;

	// If the caret is already enabled, just return
	if (m_nDisableCount == 0)
		return;

	// Check to see if we still have pending disables.
	--m_nDisableCount;
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
}

/** Determines whether Abi is going to blink the caret or not.
 * If not, then _blink() won't actually clear the caret; it'll only draw. */
void GR_Caret::setBlink(bool bBlink)
{
#ifdef XP_UNIX_TARGET_GTK
	gboolean can;
	GtkSettings * settings = gtk_settings_get_default ();

	g_object_get (G_OBJECT(settings), "gtk-cursor-blink", &can, NULL);
	m_bCursorBlink = (can != FALSE);
#elif defined(WIN32)
	m_bCursorBlink = (((int)GetCaretBlinkTime ()) > 0);
#else
	m_bCursorBlink = bBlink;
#endif
}

void GR_Caret::_erase()
{
	if (m_bCursorIsOn)
		_blink(true);
}

void GR_Caret::_blink(bool bExplicit)
{
	if (m_bRecursiveDraw || !m_bPositionSet)
		return;

	m_bRecursiveDraw = true;
	GR_Painter painter (m_pG);
	m_bRecursiveDraw = false;
	// After any autoblink, we want there to be BLINK_TIME 
	// until next autoblink.
	if (!bExplicit)
	{ 
		m_worker->stop(); m_worker->start();
	}
	if(m_bRemote)
	  {
	    xxx_UT_DEBUGMSG(("Remote Caret %x blink at x %d \n",this,m_xPoint));
	  }
	else
	  {
	    xxx_UT_DEBUGMSG(("Local Caret %x blink at x %d \n",this,m_xPoint));
	  }

	// Blink if: (a) _blink explicitly called (not autoblink); or
	//           (b) autoblink and caret blink enabled; or
	//           (c) autoblink, caret blink disabled, caret is off
	if (bExplicit || getCanCursorBlink () || !m_bCursorIsOn)
	{
		m_bRecursiveDraw = true;
		
		UT_RGBColor oldColor; m_pG->getColor(oldColor);

		if (m_bCursorIsOn)
		{
			m_pG->restoreRectangle(m_iCaretNumber*3+0);
			xxx_UT_DEBUGMSG(("blink cursor turned off \n")); 

			if(m_bSplitCaret)
			{
				m_pG->restoreRectangle(m_iCaretNumber*3+1);
				m_pG->restoreRectangle(m_iCaretNumber*3+2);
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

			// we position the regular LTR caret so that it slightly
			// covers the right edge of the character; in order to be
			// consitent we will do notionally the same for the RTL
			// carets, where this requires addition in the calculation
			// of coordinances instead of substraction; we will use
			// the following value as a sign to avoid having to branch
			// all the draw calls (Tomas, Oct 26, 2003).
			
			UT_sint32 iDelta = m_bPointDirection ? 1 : -1;
			
			UT_Rect r0(m_xPoint-m_pG->tlu(2),
					   m_yPoint+m_pG->tlu(1),
					   m_pG->tlu(5),
					   m_iPointHeight+m_pG->tlu(2));
			
			m_pG->saveRectangle(r0,m_iCaretNumber*3+0);

			if((m_xPoint != m_xPoint2) || (m_yPoint != m_yPoint2))
			{
				m_bSplitCaret = true;
				
				// have to save the rectangle for the joining line
				// before we draw the carets
				UT_uint32 xmin = UT_MIN(m_xPoint, m_xPoint2);
				UT_uint32 xmax = UT_MAX(m_xPoint, m_xPoint2);
				UT_uint32 ymin = UT_MIN(m_yPoint, m_yPoint2);
				UT_uint32 ymax = UT_MAX(m_yPoint, m_yPoint2);
			
				UT_Rect r2(xmin-m_pG->tlu(1),
						   ymin + m_iPointHeight,
						   xmax - xmin + m_pG->tlu(2),
						   ymax - ymin + m_pG->tlu(1));
				
				m_pG->saveRectangle(r2,m_iCaretNumber*3+2);
			}
			else
				m_bSplitCaret = false;

			if(m_insertMode)
				m_pG->setColor(m_clrInsert);
			else
				m_pG->setColor(m_clrOverwrite);
			if(m_bRemote)
			        m_pG->setColor(m_clrRemote);
			if(m_bCaret1OnScreen)
			{
				// draw the primary caret
				xxx_UT_DEBUGMSG(("blink cursor turned on \n")); 
				UT_sint32 x1 = m_xPoint + iDelta * m_pG->tlu(1);
				UT_sint32 x2 = m_xPoint;
				while(m_pG->_tduX(x1) == m_pG->_tduX(x2))
				{
					x1 += iDelta;
				}
				painter.drawLine(x1,
								 m_yPoint + m_pG->tlu(1),
								 x1, 
								 m_yPoint + m_iPointHeight+m_pG->tlu(1));
				
				painter.drawLine(x2,
								 m_yPoint + m_pG->tlu(1),
								 x2, 
								 m_yPoint + m_iPointHeight + m_pG->tlu(1));
			}
			
			if(m_bSplitCaret)
			{
				// each of the two carets will have a small flag at the top 
				// indicating the direction of writing to which it
				// applies
				if(m_bCaret1OnScreen)
				{
					// draw the flag of the primary caret
					if(m_bPointDirection)
					{
						//primary RTL caret flag
						painter.drawLine(m_xPoint - m_pG->tlu(2),
										 m_yPoint + m_pG->tlu(1),
										 m_xPoint /*- m_pG->tlu(1)*/,
										 m_yPoint + m_pG->tlu(1));
						
						painter.drawLine(m_xPoint - m_pG->tlu(1),
										 m_yPoint + m_pG->tlu(2),
										 m_xPoint /*- m_pG->tlu(1)*/,
										 m_yPoint+m_pG->tlu(2));
					}
					else
					{
						// primary LTR caret flag
						painter.drawLine(m_xPoint + m_pG->tlu(1),
										 m_yPoint + m_pG->tlu(1),
										 m_xPoint + m_pG->tlu(3),
										 m_yPoint + m_pG->tlu(1));
						
						painter.drawLine(m_xPoint + m_pG->tlu(1),
										 m_yPoint + m_pG->tlu(2),
										 m_xPoint + m_pG->tlu(2),
										 m_yPoint + m_pG->tlu(2));
					}
				}
				
				// Now we deal with the secondary caret needed on ltr-rtl boundary
				if(m_bCaret2OnScreen)
				{
					// first of all, save the area it will cover
					UT_Rect r1(m_xPoint2 - m_pG->tlu(2),
							   m_yPoint2 + m_pG->tlu(1),
							   m_pG->tlu(5),
							   m_iPointHeight);
					
					m_pG->saveRectangle(r1,m_iCaretNumber*3+1);				

					// draw the caret
					painter.drawLine(m_xPoint2 - iDelta * m_pG->tlu(1),
									 m_yPoint2 + m_pG->tlu(1), 
									 m_xPoint2 - iDelta * m_pG->tlu(1),
									 m_yPoint2 + m_iPointHeight + m_pG->tlu(1));
					
					painter.drawLine(m_xPoint2,
									 m_yPoint2 + m_pG->tlu(1), 
									 m_xPoint2,
									 m_yPoint2 + m_iPointHeight + m_pG->tlu(1));

					// Now draw the line that links the two carets
					painter.drawLine(m_xPoint,
									 m_yPoint + m_iPointHeight, 
									 m_xPoint2,
									 m_yPoint2 + m_iPointHeight);

					// now draw the direction flag for the secondary caret
					if(m_bPointDirection)
					{
						// secondary LTR caret flag
						painter.drawLine(m_xPoint2 + m_pG->tlu(1),
										 m_yPoint2 + m_pG->tlu(1), 
										 m_xPoint2 + m_pG->tlu(3),
										 m_yPoint2 + m_pG->tlu(1));
						
						painter.drawLine(m_xPoint2 + m_pG->tlu(1),
										 m_yPoint2 + m_pG->tlu(2), 
										 m_xPoint2 + m_pG->tlu(2),
										 m_yPoint2 + m_pG->tlu(2));
					}
					else
					{
						// secondary RTL caret flag
						painter.drawLine(m_xPoint2 - m_pG->tlu(2),
										 m_yPoint2 + m_pG->tlu(1), 
										 m_xPoint2 /*- m_pG->tlu(1)*/,
										 m_yPoint2 + m_pG->tlu(1));
						
						painter.drawLine(m_xPoint2 - m_pG->tlu(1),
										 m_yPoint2 + m_pG->tlu(2), 
										 m_xPoint2 /*- m_pG->tlu(1)*/,
										 m_yPoint2 + m_pG->tlu(2));
					}
				}
				
			}
			
		}

		m_bCursorIsOn = !m_bCursorIsOn;
 		m_pG->setColor(oldColor);
		m_bRecursiveDraw = false;
	}
	m_pG->flush();
}

/*!
 * Only call this is you are absolutely certain you need it!
 */
void GR_Caret::forceDraw(void)
{
	_blink(true);
}
//////////////////////////////////////////////////////////////////////////////////////
