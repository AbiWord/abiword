/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 



#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_NamedVirtualKey.h"


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

class ABI_EXPORT ev_EB_MouseTable
{
public:
	ev_EB_MouseTable()
		{
			reset();
		};
	
	void reset()
		{
			memset(m_peb,0,sizeof(m_peb));
		}

	~ev_EB_MouseTable()
		{
			for (UT_uint32 i=0; i < EV_COUNT_EMO; i++)
				for (UT_uint32 j=0; j < EV_COUNT_EMS; j++)
					for (UT_uint32 k=0; k< EV_COUNT_EMC; k++)
						if (m_peb[i][j][k])
							delete m_peb[i][j][k];
		}
	
	EV_EditBinding *	m_peb[EV_COUNT_EMO][EV_COUNT_EMS][EV_COUNT_EMC];
};

class ABI_EXPORT ev_EB_NVK_Table
{
public:
	ev_EB_NVK_Table()
		{
			reset();
		};
	void reset()
		{
			memset(m_peb,0,sizeof(m_peb));
		}
	~ev_EB_NVK_Table()
		{
			for (UT_sint32 i=0; i < static_cast<UT_sint32>(EV_COUNT_NVK); i++)
				for (UT_sint32 j=0; j < EV_COUNT_EMS; j++)
					if (m_peb[i][j])
						delete m_peb[i][j];
		}
	
	EV_EditBinding *	m_peb[EV_COUNT_NVK][EV_COUNT_EMS];
};

class ABI_EXPORT ev_EB_Char_Table
{
public:
	ev_EB_Char_Table()
		{
			reset();
		};
	void reset()
		{
			memset(m_peb,0,sizeof(m_peb));
		}
	~ev_EB_Char_Table()
		{
			for (UT_sint32 i=0; i < 256; i++)
				for (UT_sint32 j=0; j < EV_COUNT_EMS_NoShift; j++)
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

EV_EditBindingMap::EV_EditBindingMap(EV_EditMethodContainer * pemc):
	m_iLastMouseNo(0)
{
	UT_ASSERT(pemc);
	m_pemc = pemc;
	UT_sint32 i = 0;
	for (i=0; i<EV_COUNT_EMB; i++)
	{
	  m_pebMT[i] = (ev_EB_MouseTable*) NULL;
	}
	m_pebNVK = (ev_EB_NVK_Table*) NULL;
	m_pebChar = (ev_EB_Char_Table*) NULL;
}

EV_EditBindingMap::~EV_EditBindingMap()
{
	UT_sint32 i = 0;
	for (i=0; i<EV_COUNT_EMB; i++)
	{
		if (m_pebMT[i])
			delete m_pebMT[i];
	}

	if (m_pebNVK)
		delete m_pebNVK;

	if (m_pebChar)
		delete m_pebChar;
}

static EV_EditBits MakeMouseEditBits( UT_uint32 button, UT_uint32 op, UT_uint32 mod, UT_uint32 context )
{	
	EV_EditBits eb = 0;
	switch (button) {
		case 0: eb |= EV_EMB_BUTTON0; break;
		case 1: eb |= EV_EMB_BUTTON1; break;
		case 2: eb |= EV_EMB_BUTTON2; break;
		case 3: eb |= EV_EMB_BUTTON3; break;
		case 4: eb |= EV_EMB_BUTTON4; break;
		case 5: eb |= EV_EMB_BUTTON5; break;
		default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN); break;
	}
	eb |= EV_EMO_FromNumber( op+1 );
	eb |= EV_EMS_FromNumber( mod );
	switch (context) {
		case 0: eb |= EV_EMC_UNKNOWN; break;
		case 1: eb |= EV_EMC_TEXT; break;
		case 2: eb |= EV_EMC_LEFTOFTEXT; break;
		case 3: eb |= EV_EMC_MISSPELLEDTEXT; break;
		case 4: eb |= EV_EMC_IMAGE; break;
		case 5: eb |= EV_EMC_IMAGESIZE; break;
		case 6: eb |= EV_EMC_FIELD; break;
		case 7: eb |= EV_EMC_HYPERLINK; break;
		case 8: eb |= EV_EMC_RIGHTOFTEXT; break;
		case 9: eb |= EV_EMC_REVISION; break;
		case 10: eb |= EV_EMC_VLINE; break;
		case 11: eb |= EV_EMC_HLINE; break;
		case 12: eb |= EV_EMC_FRAME; break;
		case 13: eb |= EV_EMC_VISUALTEXTDRAG; break;
		case 14: eb |= EV_EMC_TOPCELL; break;
		case 15: eb |= EV_EMC_TOC; break;
		case 16: eb |= EV_EMC_POSOBJECT; break;
		case 17: eb |= EV_EMC_MATH; break;
		case 18: eb |= EV_EMC_EMBED; break;
		default: UT_ASSERT(UT_SHOULD_NOT_HAPPEN); break;
	}
	return eb;
}

static EV_EditBits MakeNVKEditBits( UT_uint32 mod, UT_uint32 nvk ) 
{
	return EV_EMS_FromNumberNoShift(mod) | nvk | EV_EKP_NAMEDKEY;
}
				
static EV_EditBits MakeKeyPressEditBits( UT_uint32 mod, UT_uint32 key ) 
{	
	return EV_EMS_FromNumberNoShift(mod) | key | EV_EKP_PRESS;
}

void EV_EditBindingMap::getAll( std::map<EV_EditBits,const char*>& map )
{
	// loop through mouse contexts
    for (UT_uint32 button=0; button<sizeof(m_pebMT)/sizeof(m_pebMT[0]); ++button) {
        if (m_pebMT[button]) {
            for (UT_uint32 op=0; op<sizeof(m_pebMT[0]->m_peb)/sizeof(m_pebMT[0]->m_peb[0]); ++op) {
                for (UT_uint32 mod=0; mod<sizeof(m_pebMT[0]->m_peb[0])/sizeof(m_pebMT[0]->m_peb[0][0]); ++mod) {
                    for (UT_uint32 context=0; context<sizeof(m_pebMT[0]->m_peb[0][0])/sizeof(m_pebMT[0]->m_peb[0][0][0]); ++context) {
                        EV_EditBinding* binding = m_pebMT[button]->m_peb[op][mod][context];
                        if (binding && binding->getType()==EV_EBT_METHOD) {
                            map.insert(
                                std::map<EV_EditBits,const char*>::value_type(
                                    MakeMouseEditBits( button, op, mod, context ),
                                    binding->getMethod()->getName() )
								);
                        }
                    }
                }
            }
        }
	}

	// loop through NVK's
	if (m_pebNVK) {
		for (UT_uint32 nvk=0; nvk<sizeof(m_pebNVK->m_peb)/sizeof(m_pebNVK->m_peb[0]); ++nvk) {
			for (UT_uint32 mod=0; mod<sizeof(m_pebNVK->m_peb[0])/sizeof(m_pebNVK->m_peb[0][0]); ++mod) {
				EV_EditBinding* binding = m_pebNVK->m_peb[nvk][mod];
				if (binding && binding->getType()==EV_EBT_METHOD) {
					map.insert( 
						std::map<EV_EditBits,const char*>::value_type( 
							MakeNVKEditBits( mod, nvk ),
							binding->getMethod()->getName() )
					);
				}
			}
		}
	}
	
	// loop through keypresses
	if (m_pebChar) {
		for (UT_uint32 key=0; key<sizeof(m_pebChar->m_peb)/sizeof(m_pebChar->m_peb[0]); ++key) {
			for (UT_uint32 mod=0; mod<sizeof(m_pebChar->m_peb[0])/sizeof(m_pebChar->m_peb[0][0]); ++mod) {
				EV_EditBinding* binding = m_pebChar->m_peb[key][mod];
				if (binding && binding->getType()==EV_EBT_METHOD) {
					map.insert( 
						std::map<EV_EditBits,const char*>::value_type( 
							MakeKeyPressEditBits( mod, key ),
							binding->getMethod()->getName() )
					);
				}	
			}
		}
	}
}

void EV_EditBindingMap::findEditBits( const char* szMethodName, std::vector<EV_EditBits>& list ) 
{	
	// first check if we even know the specified method
	EV_EditMethod* method = m_pemc->findEditMethodByName( szMethodName );
	if (method) {
		
		// search in mouse contexts
        for (UT_uint32 button=0; button<sizeof(m_pebMT)/sizeof(m_pebMT[0]); ++button) {
            if (m_pebMT[button]) {
                for (UT_uint32 op=0; op<sizeof(m_pebMT[0]->m_peb)/sizeof(m_pebMT[0]->m_peb[0]); ++op) {
                    for (UT_uint32 mod=0; mod<sizeof(m_pebMT[0]->m_peb[0])/sizeof(m_pebMT[0]->m_peb[0][0]); ++mod) {
                        for (UT_uint32 context=0; context<sizeof(m_pebMT[0]->m_peb[0][0])/sizeof(m_pebMT[0]->m_peb[0][0][0]); ++context) {
                            if (bindingUsesMethod( m_pebMT[button]->m_peb[op][mod][context], method )) {
                                list.push_back( MakeMouseEditBits( button, op, mod, context ) );
                            }
                        }
                    }
                }
            }
        }

		// search in NVK's
		if (m_pebNVK) {
			for (UT_uint32 nvk=0; nvk<sizeof(m_pebNVK->m_peb)/sizeof(m_pebNVK->m_peb[0]); ++nvk) {
				for (UT_uint32 mod=0; mod<sizeof(m_pebNVK->m_peb[0])/sizeof(m_pebNVK->m_peb[0][0]); ++mod) {
					if (bindingUsesMethod( m_pebNVK->m_peb[nvk][mod], method )) {
						list.push_back( MakeNVKEditBits( mod, nvk ) );
					}
				}
			}
		}
		
		// search in keypresses
		if (m_pebChar) {
			for (UT_uint32 key=0; key<sizeof(m_pebChar->m_peb)/sizeof(m_pebChar->m_peb[0]); ++key) {
				for (UT_uint32 mod=0; mod<sizeof(m_pebChar->m_peb[0])/sizeof(m_pebChar->m_peb[0][0]); ++mod) {
					if (bindingUsesMethod( m_pebChar->m_peb[key][mod], method )) {
						list.push_back( MakeKeyPressEditBits( mod, key ) );
					}
				}
			}
		}
	}
}

bool EV_EditBindingMap::bindingUsesMethod( EV_EditBinding* binding, EV_EditMethod* method ) 
{	
	return binding && binding->getType()==EV_EBT_METHOD && binding->getMethod()==method;
}

EV_EditBinding * EV_EditBindingMap::findEditBinding(EV_EditBits eb)
{
	// this handles keyboard (nvk and char) and mouse.

	if (EV_IsMouse(eb))					// mouse
	{
		UT_uint32 n_emb = EV_EMB_ToNumber(eb)-1;
		xxx_UT_DEBUGMSG(("is mouse %d binding number %d \n",eb,n_emb));
		//
		// Handle the case of accidently middle clicking during a 
		// mouse wheel scroll.
		//
		if((n_emb == 2) && ((m_iLastMouseNo == 4) || (m_iLastMouseNo == 5)))
		{
				n_emb = m_iLastMouseNo;
		} 
		m_iLastMouseNo = n_emb;
		class ev_EB_MouseTable * p = m_pebMT[n_emb];
		if (!p)
			return 0;					// no bindings of anykind for this mouse button
		UT_uint32 n_emo = EV_EMO_ToNumber(eb)-1;
		UT_uint32 n_ems = EV_EMS_ToNumber(eb);
		UT_uint32 n_emc = EV_EMC_ToNumber(eb);
		return p->m_peb[n_emo][n_ems][n_emc];

	}
	else if (EV_IsKeyboard(eb))			// a keyevent, find out what kind
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
			if (n_evk >= 256) 
			{
				if ( n_evk >= 256 && (n_evk - 65280) < 256)
					n_evk -= 65280;  // quick fix
				else
				{
					n_evk = 'a'; 
					/* in hopes that there will be 
					   'insertData' method assigned to 	
					    plain 'a'
					*/
				}
			};

			UT_uint32 n_ems = EV_EMS_ToNumberNoShift(eb);
			return m_pebChar->m_peb[n_evk][n_ems];
		}
	}
	UT_ASSERT(0);
	return 0;
}

bool EV_EditBindingMap::setBinding(EV_EditBits eb, const char * szMethodName)
{
	EV_EditMethod * pem = m_pemc->findEditMethodByName(szMethodName);
	if (!pem)
	{
	        if(strcmp(szMethodName,"NULL") == 0)
		{
		    EV_EditBinding * ev = NULL;
		    return setBinding(eb,ev);
		}
		UT_DEBUGMSG(("Unknown method name [%s] in binding table.\n",szMethodName));
		UT_ASSERT(pem);				// TODO remove this and find a better way of doing a spelling-check...
		return false;
	}

	EV_EditBinding * peb = new EV_EditBinding(pem);
	if (!peb)
		return false;

	return setBinding(eb,peb);
}

bool EV_EditBindingMap::setBinding(EV_EditBits eb, EV_EditBinding * peb)
{
	// this handles keyboard (nvk and char) and mouse.
	// return false if the given location is already bound.

	if (EV_IsMouse(eb))					// mouse
	{
		UT_uint32 n_emb = EV_EMB_ToNumber(eb)-1;
		class ev_EB_MouseTable * p = m_pebMT[n_emb];
		if (!p)
		{
			m_pebMT[n_emb] = new ev_EB_MouseTable();
			p = m_pebMT[n_emb];
			if (!p) {
				delete peb;
				return false;
			}
		}
		UT_uint32 n_emo = EV_EMO_ToNumber(eb)-1;
		UT_uint32 n_ems = EV_EMS_ToNumber(eb);
		UT_uint32 n_emc = EV_EMC_ToNumber(eb);
		if (p->m_peb[n_emo][n_ems][n_emc]) {
			delete peb;
			return false;
		}
		p->m_peb[n_emo][n_ems][n_emc] = peb;
		return true;
	}
	else if (EV_IsKeyboard(eb))			// a keyevent, find out what kind
	{
		if (eb & EV_EKP_NAMEDKEY)		// nvk
		{
			if (!m_pebNVK)
			{
				m_pebNVK = new ev_EB_NVK_Table();
				if (!m_pebNVK) {
					delete peb;
					return false;
				}
			}
			UT_uint32 n_nvk = EV_NVK_ToNumber(eb);
			UT_uint32 n_ems = EV_EMS_ToNumber(eb);
			if (m_pebNVK->m_peb[n_nvk][n_ems]) {
				delete peb;
				return false;
			}
			m_pebNVK->m_peb[n_nvk][n_ems] = peb;
			return true;
		}
		else							// a non-nvk -- regular char
		{
			if (!m_pebChar)
			{
				m_pebChar = new ev_EB_Char_Table();
				if (!m_pebChar) {
					delete peb;
					return false;
				}
			}
			UT_uint32 n_evk = EV_EVK_ToNumber(eb);
			UT_ASSERT(n_evk < 256);		// TODO see note [1] above.
			UT_uint32 n_ems = EV_EMS_ToNumberNoShift(eb);
			if (m_pebChar->m_peb[n_evk][n_ems]) 
			{
			        UT_DEBUGMSG(("Removing and Deleting previous keybinding %p \n",m_pebChar->m_peb[n_evk][n_ems]));
				delete m_pebChar->m_peb[n_evk][n_ems];
			}
			m_pebChar->m_peb[n_evk][n_ems] = peb;
			return true;
		}
	}
	delete peb;
	UT_ASSERT(0);
	return 0;
}

bool EV_EditBindingMap::removeBinding(EV_EditBits eb)
{
	// this handles keyboard (nvk and char) and mouse.
	// remove the binding from the map.
	// return true if binding updated.
	// we do not g_free the unreferenced binding.
	
	if (EV_IsMouse(eb))					// mouse
	{
		UT_uint32 n_emb = EV_EMB_ToNumber(eb)-1;
		class ev_EB_MouseTable * p = m_pebMT[n_emb];
		if (!p)
			return false;
		UT_uint32 n_emo = EV_EMO_ToNumber(eb)-1;
		UT_uint32 n_ems = EV_EMS_ToNumber(eb);
		UT_uint32 n_emc = EV_EMC_ToNumber(eb);
		p->m_peb[n_emo][n_ems][n_emc] = 0;
		return true;
	}
	else if (EV_IsKeyboard(eb))			// a keyevent, find out what kind
	{
		if (eb & EV_EKP_NAMEDKEY)		// nvk
		{
			if (!m_pebNVK)
				return false;
			UT_uint32 n_nvk = EV_NVK_ToNumber(eb);
			UT_uint32 n_ems = EV_EMS_ToNumber(eb);
			m_pebNVK->m_peb[n_nvk][n_ems] = 0;
			return true;
		}
		else							// a non-nvk -- regular char
		{
			if (!m_pebChar)
				return false;
			UT_uint32 n_evk = EV_EVK_ToNumber(eb);
			UT_ASSERT(n_evk < 256);		// TODO see note [1] above.
			UT_uint32 n_ems = EV_EMS_ToNumberNoShift(eb);
			m_pebChar->m_peb[n_evk][n_ems] = 0;
			return true;
		}
	}
	UT_ASSERT(0);
	return 0;
}

void EV_EditBindingMap::resetAll()
{
	// NOTE: this to me seems like a memory leak,
	// but since removeBinding is so seemingly easy with leaking
	// memory as well, I just copy it's MO.
	for (size_t i=0; i<sizeof(m_pebMT)/sizeof(m_pebMT[0]); ++i) {
		m_pebMT[i]->reset();
	}
	m_pebNVK->reset();
	m_pebChar->reset();
}

const char * EV_EditBindingMap::getShortcutFor(const EV_EditMethod * pEM) const
{
	UT_ASSERT(pEM);
	if(!m_pebChar)
	  return NULL;
	// lookup the keyboard shortcut bound to pEM, if any

	EV_EditModifierState ems = 0;
	EV_EditBinding * pEB;
	UT_sint32 i, j;
	char shortcut = 0;

	// search characters first
	bool bChar = false;

	/* we lookup the table in decreasing order to be able to catch lowercase
	 BEFORE uppercase. Uppercase = Shift modifier. That is the rule */
	if (m_pebChar) 
	{
		for (i=255; (i >= 0) && !bChar; i--)
		{
			for (j=0; j < EV_COUNT_EMS_NoShift; j++)
			{
				if (m_pebChar->m_peb[i][j])
				{
					// only check non-null entries
					pEB = m_pebChar->m_peb[i][j];

					if ((pEB->getType() == EV_EBT_METHOD) && 
						(pEB->getMethod() == pEM))
					{
						// bingo
						bChar = true;
						shortcut = i;

						ems = EV_EMS_FromNumberNoShift(j);
						break;
					}
				}
			}
		}
	}
	
	bool bNVK = false;

	if (!bChar && m_pebNVK)
	{
		// then search NVKs
		for (i=0; (i < static_cast<UT_sint32>(EV_COUNT_NVK)) && !bNVK; i++)
		{
			for (j=0; j < EV_COUNT_EMS; j++)
			{
				if (m_pebNVK->m_peb[i][j])
				{
					// only check non-null entries
					pEB = m_pebNVK->m_peb[i][j];

					if ((pEB->getType() == EV_EBT_METHOD) && 
						(pEB->getMethod() == pEM))
					{
						// bingo
						bNVK = true;
						shortcut = i;

						ems = EV_EMS_FromNumber(j);
						break;
					}
				}
			}
		}
	}
	
	
	if (!bChar && !bNVK) 
	  return (const char *) NULL;

	// translate into displayable string
	static char buf[128];
	memset(buf,0,G_N_ELEMENTS(buf));

	if (ems&EV_EMS_CONTROL)
		strcat(buf, "Ctrl+");

	if (ems&EV_EMS_SHIFT)
		strcat(buf, "Shift+");

	if (ems&EV_EMS_ALT)
		strcat(buf, "Alt+");

	if (bChar)
	{
		if ((shortcut >= 'A') && (shortcut <= 'Z')) {
			/* always return an uppercase letter for the shortcut, unlike the mapper do */			
			if (!(ems&EV_EMS_SHIFT)) {
				strcat(buf, "Shift+");
			}
		}
		 else
			shortcut = toupper (shortcut); 
	            
		int len = strlen(buf);
		buf[len] = shortcut;
	}
	else
	{
		// translate NVK
	  const char * szNVK = (const char *) NULL;

		// TODO: look these up from table, rather than switch
		switch(EV_NamedKey(shortcut))
		{
		case EV_NVK_DELETE:
			szNVK = "Del";
			break;

		case EV_NVK_F1:
			szNVK = "F1";
			break;

		case EV_NVK_F3:
			szNVK = "F3";
			break;

		case EV_NVK_F4:
			szNVK = "F4";
			break;

		case EV_NVK_F7:
			szNVK = "F7";
			break;

		case EV_NVK_F10:
			szNVK = "F10";
			break;

		case EV_NVK_F11:
			szNVK = "F11";
			break;

		case EV_NVK_F12:
			szNVK = "F12";
			break;

		default:
			szNVK = "unmapped NVK";
			break;
		}

		strcat(buf, szNVK);
	}

	return buf;
}
	
bool EV_EditBindingMap::parseEditBinding(void /*const char * szAscii*/)
{
	/* TODO here we import a binding from a primitive ascii format
	** TODO or XML syntax.
	*/
  /*
    The Ascii format is close to the definition in wp/ap/xp/ap_LB_Default.cpp
Lines beginning with "//" are comments

Each CR seperated lines define a set of bindings for a single mouse context or
key stroke. Entries are seperated by commas. In creating the context, several
modifiers can be used. Control, Alt, Shift, Control-Alt (C, A, S )

The first token of each line determines whether the defintion is for mouse context (mse), Named Virtual Key (nvk), or keystroke (key).

The definition of each set of bindings are always in the following order for
mouse contexts.

Up to to 6 buttons are available for the mouse. (B0, B1, B2, B3, B4, B5, B6)
The follow contexts are available:

Short cut       C++ enum
=========       =========
CU		EV_EMC_UNKNOWN
CT		EV_EMC_TEXT
CM		EV_EMC_MISSPELLEDTEXT
CL		EV_EMC_LEFTOFTEXT
CR		EV_EMC_RIGHTOFTEXT
CI		EV_EMC_IMAGE
CZ		EV_EMC_IMAGESIZE
CF		EV_EMC_FIELD
CH		EV_EMC_HYPERLINK
CV		EV_EMC_REVISION
CTV		EV_EMC_VLINE
CTH		EV_EMC_HLINE
CTF		EV_EMC_FRAME
CVD		EV_EMC_VISUALTEXTDRAG
CTC		EV_EMC_TOPCELL
CTO		EV_EMC_TOC
CPO		EV_EMC_POSOBJECT
CMA		EV_EMC_MATH
CEM		EV_EMC_EMBED

mse, Button, context, click, dblclick, drag, dbldrag, release, double release

The first 3 entries describe the combination of buttons and context, the six entries that follow are the names of the EditMethods that called for each invocation of the mouse button and context.

So some examples are:

mse, B0,      CU   ,        ,       , cursorDefault,  ,  ,
mse, B0,     CEM   ,        ,       , btn0InlineImage ,  ,
mse, B1,     CVD   , cutVisualText, copyVisualText, dragVisualText, dragVisualText, dragVisualText, pasteVisualText
mse, B1, CVD C     ,copyVisualText,cutVisualText,dragVisualText,dragVisualText, pasteVisualText,pasteVisualText

//
// mse/NVK/Key
// NVK, Key Name,    No modifier, S,     C,  S C,A , A S, A C, A C S
//
NVK, EV_NVK_BACKSPACE, delLeft,delLeft,delBOW,  ,   ,   ,    ,
//
// key, Key Value,    No modifier,   C,       A,    A C
//
key,    0x41,         insertData, selectAll,      , 
  */

	return false;
}
