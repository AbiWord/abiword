/* AbiWord
 * Copyright (c) 2003 Dom Lachowicz
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "fv_View.h"
#include "fl_SelectionPreserver.h"

FL_SelectionPreserver::FL_SelectionPreserver (FV_View * pView)
	: m_pView (pView), m_bHadSelection (false)
{
	UT_ASSERT (pView);
	
	// cache whether we had the selection, and what the selection is
	if (!pView->isSelectionEmpty ()) {
		m_bHadSelection = true;
		pView->getDocumentRangeOfCurrentSelection (&m_docRange);
	}
}

FL_SelectionPreserver::~FL_SelectionPreserver ()
{
	// restore the selection if we once had it
	// TODO: this might not be entirely correct, but it's probably
	// a "close enough" heuristic for this class' uses
	if (m_bHadSelection) {
		m_pView->cmdUnselectSelection();
		m_pView->cmdSelect (m_docRange.m_pos1, m_docRange.m_pos2);
	}
}

bool FL_SelectionPreserver::cmdCharInsert(const UT_UCSChar * text, UT_uint32 count, bool bForce)
{
	// TODO: how to handle bForce???
	m_docRange.m_pos2 += count;
	if (!m_pView->isSelectionEmpty ())
	{
		// the code here used to call getSelectionText() to ascertain the length of the selection --
		// that is very inefficient as the function clones the selection text.
		m_docRange.m_pos2 -= m_pView->getSelectionLength();
	}
	
	return m_pView->cmdCharInsert (text, count, bForce);
}
