/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 



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
												   EV_EditMethod ** ppEM,
												   UT_uint32 * piPrefixCount)
{
	// TODO actually compute prefix count

	UT_ASSERT(ppEM);
	UT_ASSERT(piPrefixCount);

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
		*piPrefixCount = 1;
		m_pebmInProgress = 0;
		return EV_EEMR_COMPLETE;

	default:
		UT_ASSERT(0);
		m_pebmInProgress = 0;
		return EV_EEMR_BOGUS_START;
	}
}

EV_EditEventMapperResult EV_EditEventMapper::Keystroke(EV_EditBits eb,
													   EV_EditMethod ** ppEM,
													   UT_uint32 * piPrefixCount)
{
	// TODO actually compute prefix count

	UT_ASSERT(ppEM);
	UT_ASSERT(piPrefixCount);

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
		*piPrefixCount = 1;
		m_pebmInProgress = 0;
		return EV_EEMR_COMPLETE;

	default:
		UT_ASSERT(0);
		m_pebmInProgress = 0;
		return EV_EEMR_BOGUS_START;
	}
}



