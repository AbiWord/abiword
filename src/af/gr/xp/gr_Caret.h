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
#ifndef GR_CARET_H
#define GR_CARET_H

#include "ut_timer.h"
#include "ut_misc.h"
#include "ut_assert.h"

class GR_Graphics;

class ABI_EXPORT GR_Caret
{
public:
	explicit GR_Caret(GR_Graphics * pG);
	~GR_Caret();
	
	void enable();
	// Disable hides the caret if it's currently visible.
	// The caret can be disabled multiple times, and just as many enable
	// calls need to take place for the cursor to show up.
	// If the cursor is already enabled, the enable call just makes the
	// cursor get redrawn.
	// if bNoMulti is true, then only one enable call needed to reverse this disable.
	void disable(bool bNoMulti = false);
	inline bool isEnabled() const { return m_nDisableCount == 0; }
	
	void setBlink(bool bBlink);

	// When you call setCoords, the cursor is explicitly shown
	// and the timer restarts from 0 for the next 500ms cycle.
	void setCoords(UT_sint32 x, UT_sint32 y, UT_uint32 h,
				   UT_sint32 x2 = 0, UT_sint32 y2 = 0, UT_uint32 h2 = 0,
				   bool bPointDirection = false, UT_RGBColor * pClr = NULL);
private:
	static void s_work(UT_Worker * w);
	static void s_enable(UT_Worker * w);

	UT_sint32			m_xPoint;
	UT_sint32			m_yPoint;
	UT_uint32			m_iPointHeight;
    // for bidi
	UT_sint32			m_xPoint2;
	UT_sint32			m_yPoint2;
	UT_uint32			m_iPointHeight2;
	bool				m_bPointDirection;
	UT_RGBColor *		m_pClr;
	GR_Graphics *		m_pG;

	GR_Caret(); // no impl
	GR_Caret(const GR_Caret& rhs);			// no impl.
	void operator=(const GR_Caret& rhs);	// no impl.
	
	void	_erase();
	void	_blink(bool bExplicit);
	
	UT_Timer *    m_worker;
	UT_Timer *	  m_enabler;

	// m_nDisableCount > 0 implies a disabled cursor.
	// m_nDisableCount should never be negative.
	UT_uint32	m_nDisableCount;
	bool		m_bCursorBlink;
	bool		m_bCursorIsOn;
	bool		m_bPositionSet;
	bool		m_bRecursiveDraw;
	bool        m_bSplitCaret;
	bool        m_bCaret1OnScreen;
	bool        m_bCaret2OnScreen;
};

class GR_CaretDisabler
{
 public:

	GR_CaretDisabler (GR_Caret * pCaret)
		: m_pCaret(pCaret)
		{
			if (m_pCaret) 
			{
				m_pCaret->disable();
			}
		}

	~GR_CaretDisabler ()
		{
			if(m_pCaret)
			{
				m_pCaret->enable();
			}
		}

private:
	// no impls
	GR_CaretDisabler ();
	GR_CaretDisabler (const GR_CaretDisabler & other);
	GR_CaretDisabler & operator=(const GR_CaretDisabler &other);
	
	GR_Caret * m_pCaret;
};


#endif // GR_CARET_H
