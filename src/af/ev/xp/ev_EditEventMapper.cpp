 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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



