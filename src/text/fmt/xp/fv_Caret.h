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
#ifndef FV_CARET_H
#define FV_CARET_H

#include "ut_thread.h"
#include "ut_mutex.h"

class FV_View;

//////////////////////////////////////////////////////////////////
// The following class is used internally by FV_Cursor
// It should possibly (probably) be moved into its own file(s), but for
// now, during development, it's left here due to the close relationship
// with FV_Cursor.

class FV_Caret
{
public:

	explicit FV_Caret(FV_View& view);
	~FV_Caret();
	
	void enable();
	void disable();
	
	inline bool isEnabled() const { return m_nDisableCount == 0; }
	
	void setBlink(bool bBlink);

private:

	class Worker : public UT_Thread
	{
		// This [inner] class is here for two reasons.
		// 1. To inhibit any function but the one in the
		//    friend declaration to make the caret blink.
		// 2. To display how friendship actually can strengthen
		//    encapsulation (compared to giving this free function
		//    friendship to the class FV_Caret)..
	public:
		explicit Worker (FV_Caret & owner)
			: UT_Thread(), m_bDie(false), m_owner (owner)
			{
			}

		virtual ~Worker ()
			{
			}

		bool m_bDie;

	protected:

		virtual void run () ;

	private:

		// no impls
		Worker () ;
		Worker ( const Worker & rhs ) ;
		Worker & operator= ( const Worker & rhs ) ;

		FV_Caret & m_owner;
	};

	friend Worker;

	FV_Caret(); // no impl
	FV_Caret(const FV_Caret& rhs);			// no impl.
	void operator=(const FV_Caret& rhs);	// no impl.
	
	void	_blink();
	
	FV_View&			m_view;

	Worker      m_worker;

	UT_Mutex    m_disableProtector;
	UT_uint32	m_nDisableCount;
	bool				m_bCursorBlink;
	bool				m_bCursorIsOn;
};

#endif // FV_CARET_H
