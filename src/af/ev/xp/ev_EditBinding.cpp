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
 



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_NamedVirtualKey.h"


#define NrElements(a)	((sizeof(a) / sizeof(a[0])))

/*****************************************************************/
/*****************************************************************/

EV_EditBinding::EV_EditBinding(EV_EditBindingMap * pebm)
{
	m_ebt = EV_EBT_PREFIX;
	u.m_pebm = pebm;
}

EV_EditBinding::EV_EditBinding(EV_EditMethod * pem)
{
	m_ebt = EV_EBT_METHOD;
	u.m_pem = pem;
}

EV_EditBindingType EV_EditBinding::getType(void) const
{
	return m_ebt;
}

EV_EditBindingMap * EV_EditBinding::getMap(void) const
{
	UT_ASSERT(m_ebt == EV_EBT_PREFIX);
	return u.m_pebm;
}

EV_EditMethod * EV_EditBinding::getMethod(void) const
{
	UT_ASSERT(m_ebt == EV_EBT_METHOD);
	return u.m_pem;
}

/*****************************************************************/
/*****************************************************************/

class ev_EB_MouseTable
{
public:
	ev_EB_MouseTable()
		{
			memset(m_peb,0,sizeof(m_peb));
		};

	~ev_EB_MouseTable()
		{
			for (UT_uint32 i=0; i < EV_COUNT_EMO; i++)
				for (UT_uint32 j=0; j < EV_COUNT_EMS; j++)
					if (m_peb[i][j])
						delete m_peb[i][j];
		}
	
	EV_EditBinding *	m_peb[EV_COUNT_EMO][EV_COUNT_EMS];
};

class ev_EB_NVK_Table
{
public:
	ev_EB_NVK_Table()
		{
			memset(m_peb,0,sizeof(m_peb));
		};
	~ev_EB_NVK_Table()
		{
			for (UT_uint32 i=0; i < EV_COUNT_NVK; i++)
				for (UT_uint32 j=0; j < EV_COUNT_EMS; j++)
					if (m_peb[i][j])
						delete m_peb[i][j];
		}
	
	EV_EditBinding *	m_peb[EV_COUNT_NVK][EV_COUNT_EMS];
};

class ev_EB_Char_Table
{
public:
	ev_EB_Char_Table()
		{
			memset(m_peb,0,sizeof(m_peb));
		};
	~ev_EB_Char_Table()
		{
			for (UT_uint32 i=0; i < 256; i++)
				for (UT_uint32 j=0; j < EV_COUNT_EMS_NoShift; j++)
					if (m_peb[i][j])
						delete m_peb[i][j];
		}
	

	// TODO Note[1]  we currently limit the range on regular (non-nvk)
	// TODO Note[1]  keys to 256.  This is probably OK for Latin1, but
	// TODO Note[1]  will probably need to be re-addressed later.
	
	EV_EditBinding *	m_peb[256][EV_COUNT_EMS_NoShift];
};

/*****************************************************************/
/*****************************************************************/

EV_EditBindingMap::EV_EditBindingMap(EV_EditMethodContainer * pemc)
{
	UT_ASSERT(pemc);
	m_pemc = pemc;
	for (int i=0; i<EV_COUNT_EMB; i++)
	{
		m_pebMT[i] = NULL;
	}
	m_pebNVK = NULL;
	m_pebChar = NULL;
}

EV_EditBindingMap::~EV_EditBindingMap()
{
	for (UT_uint32 i=0; i<EV_COUNT_EMB; i++)
	{
		if (m_pebMT[i])
			delete m_pebMT[i];
	}

	if (m_pebNVK)
		delete m_pebNVK;

	if (m_pebChar)
		delete m_pebChar;
}

EV_EditBinding * EV_EditBindingMap::findEditBinding(EV_EditBits eb)
{
	// this handles keyboard (nvk and char) and mouse.

	if (eb & EV_EMB__MASK__)			// mouse
	{
		UT_uint32 n_emb = EV_EMB_ToNumber(eb)-1;
		class ev_EB_MouseTable * p = m_pebMT[n_emb];
		if (!p)
			return 0;					// no bindings of anykind for this mouse button
		UT_uint32 n_emo = EV_EMO_ToNumber(eb)-1;
		UT_uint32 n_ems = EV_EMS_ToNumber(eb);
		return p->m_peb[n_emo][n_ems];
	}
	else if (eb & EV_EKP_PRESS)			// a keyevent, find out what kind
	{
		if (eb & EV_EKP_NAMEDKEY)		// a NVK
		{
			if (!m_pebNVK)
				return 0;				// no bindings of anykind for nvk keys
			
			UT_uint32 n_nvk = EV_NVK_ToNumber(eb);
			UT_uint32 n_ems = EV_EMS_ToNumber(eb);
			return m_pebNVK->m_peb[n_nvk][n_ems];
		}
		else							// not a NVK -- regular char
		{
			if (!m_pebChar)
				return 0;				// no bindings of anykind for non-nvk keys

			UT_uint32 n_evk = EV_EVK_ToNumber(eb);
			UT_ASSERT(n_evk < 256);		// TODO see note [1] above.
			UT_uint32 n_ems = EV_EMS_ToNumberNoShift(eb);
			return m_pebChar->m_peb[n_evk][n_ems];
		}
	}
	UT_ASSERT(0);
	return 0;
}

UT_Bool EV_EditBindingMap::setBinding(EV_EditBits eb, const char * szMethodName)
{
	EV_EditMethod * pem = m_pemc->findEditMethodByName(szMethodName);
	UT_ASSERT(pem);						// TODO remove this and find a better way of doing a spelling-check...
	
	if (!pem)
		return UT_FALSE;
	EV_EditBinding * peb = new EV_EditBinding(pem);
	if (!peb)
		return UT_FALSE;

	return setBinding(eb,peb);
}

UT_Bool EV_EditBindingMap::setBinding(EV_EditBits eb, EV_EditBinding * peb)
{
	// this handles keyboard (nvk and char) and mouse.
	// return false if the given location is already bound.

	if (eb & EV_EMB__MASK__)			// mouse
	{
		UT_uint32 n_emb = EV_EMB_ToNumber(eb)-1;
		class ev_EB_MouseTable * p = m_pebMT[n_emb];
		if (!p)
		{
			m_pebMT[n_emb] = new ev_EB_MouseTable();
			p = m_pebMT[n_emb];
			if (!p)
				return UT_FALSE;
		}
		UT_uint32 n_emo = EV_EMO_ToNumber(eb)-1;
		UT_uint32 n_ems = EV_EMS_ToNumber(eb);
		if (p->m_peb[n_emo][n_ems])
			return UT_FALSE;
		p->m_peb[n_emo][n_ems] = peb;
		return UT_TRUE;
	}
	else if (eb & EV_EKP_PRESS)			// a keyevent, find out what kind
	{
		if (eb & EV_EKP_NAMEDKEY)		// nvk
		{
			if (!m_pebNVK)
			{
				m_pebNVK = new ev_EB_NVK_Table();
				if (!m_pebNVK)
					return UT_FALSE;
			}
			UT_uint32 n_nvk = EV_NVK_ToNumber(eb);
			UT_uint32 n_ems = EV_EMS_ToNumber(eb);
			if (m_pebNVK->m_peb[n_nvk][n_ems])
				return UT_FALSE;
			m_pebNVK->m_peb[n_nvk][n_ems] = peb;
			return UT_TRUE;
		}
		else							// a non-nvk -- regular char
		{
			if (!m_pebChar)
			{
				m_pebChar = new ev_EB_Char_Table();
				if (!m_pebChar)
					return UT_FALSE;
			}
			UT_uint32 n_evk = EV_EVK_ToNumber(eb);
			UT_ASSERT(n_evk < 256);		// TODO see note [1] above.
			UT_uint32 n_ems = EV_EMS_ToNumberNoShift(eb);
			if (m_pebChar->m_peb[n_evk][n_ems])
				return UT_FALSE;
			m_pebChar->m_peb[n_evk][n_ems] = peb;
			return UT_TRUE;
		}
	}
	UT_ASSERT(0);
	return 0;
}

UT_Bool EV_EditBindingMap::removeBinding(EV_EditBits eb)
{
	// this handles keyboard (nvk and char) and mouse.
	// remove the binding from the map.
	// return true if binding updated.
	// we do not free the unreferenced binding.
	
	if (eb & EV_EMB__MASK__)			// mouse
	{
		UT_uint32 n_emb = EV_EMB_ToNumber(eb)-1;
		class ev_EB_MouseTable * p = m_pebMT[n_emb];
		if (!p)
			return UT_FALSE;
		UT_uint32 n_emo = EV_EMO_ToNumber(eb)-1;
		UT_uint32 n_ems = EV_EMS_ToNumber(eb);
		p->m_peb[n_emo][n_ems] = 0;
		return UT_TRUE;
	}
	else if (eb & EV_EKP_PRESS)			// a keyevent, find out what kind
	{
		if (eb & EV_EKP_NAMEDKEY)		// nvk
		{
			if (!m_pebNVK)
				return UT_FALSE;
			UT_uint32 n_nvk = EV_NVK_ToNumber(eb);
			UT_uint32 n_ems = EV_EMS_ToNumber(eb);
			m_pebNVK->m_peb[n_nvk][n_ems] = 0;
			return UT_TRUE;
		}
		else							// a non-nvk -- regular char
		{
			if (!m_pebChar)
				return UT_FALSE;
			UT_uint32 n_evk = EV_EVK_ToNumber(eb);
			UT_ASSERT(n_evk < 256);		// TODO see note [1] above.
			UT_uint32 n_ems = EV_EMS_ToNumberNoShift(eb);
			m_pebChar->m_peb[n_evk][n_ems] = 0;
			return UT_TRUE;
		}
	}
	UT_ASSERT(0);
	return 0;
}

const char * EV_EditBindingMap::getShortcutFor(const EV_EditMethod * pEM) const
{
	UT_ASSERT(pEM);

	// lookup the keyboard shortcut bound to pEM, if any

	EV_EditModifierState ems;
	EV_EditBinding * pEB;
	UT_uint32 i, j;

	// search characters first
	UT_Bool bChar = UT_FALSE;

	for (i=0; (i < 256) && !bChar; i++)
		for (j=0; j < EV_COUNT_EMS_NoShift; j++)
			if (m_pebChar->m_peb[i][j])
			{
				// only check non-null entries
				pEB = m_pebChar->m_peb[i][j];

				if ((pEB->getType() == EV_EBT_METHOD) && 
					(pEB->getMethod() == pEM))
				{
					// bingo
					bChar = UT_TRUE;

					ems = EV_EMS_FromNumberNoShift(j);
					break;
				}
			}

	UT_Bool bNVK = UT_FALSE;

	if (!bChar)
	{
		// then search NVKs
		for (i=0; (i < EV_COUNT_NVK) && !bNVK; i++)
			for (j=0; j < EV_COUNT_EMS; j++)
				if (m_pebNVK->m_peb[i][j])
				{
					// only check non-null entries
					pEB = m_pebNVK->m_peb[i][j];

					if ((pEB->getType() == EV_EBT_METHOD) && 
						(pEB->getMethod() == pEM))
					{
						// bingo
						bNVK = UT_TRUE;

						ems = EV_EMS_FromNumber(j);
						break;
					}
				}
	}
	
	
	if (!bChar && !bNVK) 
		return NULL;

	// translate into displayable string
	static char buf[128];
	memset(buf,0,NrElements(buf));

	if (ems&EV_EMS_CONTROL)
		strcat(buf, "Ctrl+");

	if (ems&EV_EMS_SHIFT)
		strcat(buf, "Shift+");

	if (ems&EV_EMS_ALT)
		strcat(buf, "Alt+");

	if (bChar)
	{
		int len = strlen(buf);
		buf[len] = (char)(i-1);
	}
	else
	{
		// translate NVK
		const char * szNVK = NULL;

		// TODO: look these up from table, rather than switch
		switch(EV_NamedKey(i-1))
		{
		case EV_NVK_DELETE:
			szNVK = "Del";
			break;

		case EV_NVK_F4:
			szNVK = "F4";
			break;

		default:
			szNVK = "unmapped NVK";
			break;
		}

		strcat(buf, szNVK);
	}

	return buf;
}
	
UT_Bool EV_EditBindingMap::parseEditBinding(void)
{
	/* TODO here we import a binding from a primitive ascii format
	** TODO or XML syntax.
	*/
	return UT_FALSE;
}
