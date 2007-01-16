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

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"


EV_EditEventMapper::EV_EditEventMapper(EV_EditBindingMap * pebm)
{
	UT_ASSERT(pebm);
	m_pebmTopLevel = pebm;

	m_pebmInProgress = 0;
}

EV_EditEventMapperResult EV_EditEventMapper::Mouse(EV_EditBits eb,
												   EV_EditMethod ** ppEM)
{
	UT_ASSERT(ppEM);

#if 0
	{
		char *context[]= { "", "Text", "LeftOfText", "RightOfText", "Image", "ImageSize" };
		char *ops[]    = { "", "Click", "DblClick", "Drag", "DblDrag", "Release", "DblRelease" };
		char *keys[]   = { "", "Shift", "Control", "ShiftControl", "Alt", "ShiftAlt", "ControlAlt" "ShiftControlAlt" };
		UT_DEBUGMSG(("Mouse: [context %s][op %s][button %d][keys %s]\n",
					 context[EV_EMC_ToNumber(eb)],
					 ops[EV_EMO_ToNumber(eb)],
					 EV_EMB_ToNumber(eb)-1,
					 keys[EV_EMS_ToNumber(eb)]));
	}
#endif
				 
	if (!m_pebmInProgress)
		m_pebmInProgress = m_pebmTopLevel;

	EV_EditBinding * peb = m_pebmInProgress->findEditBinding(eb);
	if (!peb)							// bogus key
	{
		EV_EditEventMapperResult r = (  (m_pebmInProgress==m_pebmTopLevel)
									  ? EV_EEMR_BOGUS_START
									  : EV_EEMR_BOGUS_CONT);
		m_pebmInProgress = 0;
		return r;
	}

	EV_EditBindingType t = peb->getType();
	switch (t)
	{
	case EV_EBT_PREFIX:
		m_pebmInProgress = peb->getMap();
		UT_ASSERT(m_pebmInProgress);
		return EV_EEMR_INCOMPLETE;

	case EV_EBT_METHOD:
		*ppEM = peb->getMethod();
		m_pebmInProgress = 0;
		return EV_EEMR_COMPLETE;

	default:
		UT_ASSERT(0);
		m_pebmInProgress = 0;
		return EV_EEMR_BOGUS_START;
	}
}

EV_EditEventMapperResult EV_EditEventMapper::Keystroke(EV_EditBits eb,
													   EV_EditMethod ** ppEM)
{
	UT_ASSERT(ppEM);

	if (!m_pebmInProgress)
		m_pebmInProgress = m_pebmTopLevel;

	EV_EditBinding * peb = m_pebmInProgress->findEditBinding(eb);
	if (!peb)							// bogus key
	{
		EV_EditEventMapperResult r = (  (m_pebmInProgress==m_pebmTopLevel)
									  ? EV_EEMR_BOGUS_START
									  : EV_EEMR_BOGUS_CONT);
		m_pebmInProgress = 0;
		return r;
	}

	EV_EditBindingType t = peb->getType();
	switch (t)
	{
	case EV_EBT_PREFIX:
		m_pebmInProgress = peb->getMap();
		UT_ASSERT(m_pebmInProgress);
		return EV_EEMR_INCOMPLETE;

	case EV_EBT_METHOD:
		*ppEM = peb->getMethod();
		m_pebmInProgress = 0;
		return EV_EEMR_COMPLETE;

	default:
		UT_ASSERT(0);
		m_pebmInProgress = 0;
		return EV_EEMR_BOGUS_START;
	}
}

const char * EV_EditEventMapper::getShortcutFor(const EV_EditMethod * pEM) const
{
	UT_ASSERT(pEM);

	// lookup the keyboard shortcut bound to pEM, if any

	return m_pebmTopLevel->getShortcutFor(pEM);
}
