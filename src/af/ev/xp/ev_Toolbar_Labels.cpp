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
 


#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_Toolbar_Labels.h"
#ifdef BIDI_ENABLED
#include "fribidi.h"
#include "xap_EncodingManager.h"
#include "xap_App.h"
#endif

/*****************************************************************/

EV_Toolbar_Label::EV_Toolbar_Label(XAP_Toolbar_Id id,
								   const char * szToolbarLabel,
								   const char * szIconName,
								   const char * szToolTip,
								   const char * szStatusMsg)
{

	m_id = id;
	UT_cloneString(m_szToolbarLabel,szToolbarLabel);
	UT_cloneString(m_szIconName,szIconName);
	UT_cloneString(m_szToolTip,szToolTip);
	UT_cloneString(m_szStatusMsg,szStatusMsg);

#ifdef BIDI_ENABLED	
	// TODO I wish we did not have to do this here, but I see no other
	// way; the menu mechanism is much cleaner and I think we should be
	// using the string-set mechanism for toolbars as we do for menus.
	// when/if we do, this whole bit can be removed
	if(!XAP_App::getApp()->theOSHasBidiSupport())
	{
        UT_uint32 iOldLen = 0;
        FriBidiChar *fbdStr = 0, *fbdStr2 = 0;
        char * szStr = m_szToolTip;
        for(UT_uint32 j = 0; j < 2; j++)
        {
			if (szStr && *szStr)
			{	
				UT_uint32 iStrLen  = strlen(szStr);
	
				if(iStrLen > iOldLen)
				{
					if(fbdStr)
					{
						delete [] fbdStr;
						delete [] fbdStr2;
					}
					
					fbdStr   = new FriBidiChar [iStrLen];
					UT_ASSERT(fbdStr);
					fbdStr2  = new FriBidiChar [iStrLen];
					UT_ASSERT(fbdStr2);
					iOldLen = iStrLen;
				}
				
				UT_uint32 i;
				for(i = 0; i < iStrLen; i++)
					fbdStr[i] = (FriBidiChar) XAP_EncodingManager::get_instance()->nativeToU((UT_UCSChar)szStr[i]);
	
				FriBidiCharType fbdDomDir = fribidi_get_type(fbdStr[0]);
#if 1
// this has been crashing with en-US (but not en-GB), due to some
// weird memory managment in the fribidi library; so I defined
// USE_SIMPLE_MALLOC for it, which solved the problem
				fribidi_log2vis (		/* input */
				       fbdStr,
				       iStrLen,
				       &fbdDomDir,
			    	   /* output */
				       fbdStr2,
				       NULL,
				       NULL,
				       NULL);	
#endif	
				for(i = 0; i < iStrLen; i++)
					szStr[i] = (char) XAP_EncodingManager::get_instance()->UToNative((UT_UCSChar)fbdStr2[i]);
				UT_ASSERT(szStr[i] == 0);
			}
			
			szStr = m_szStatusMsg;
		}
		
		delete[] fbdStr;
		delete[] fbdStr2;
	}
	
#endif
}

EV_Toolbar_Label::~EV_Toolbar_Label(void)
{
	FREEP(m_szToolbarLabel);
	FREEP(m_szIconName);
	FREEP(m_szToolTip);
	FREEP(m_szStatusMsg);
}

XAP_Toolbar_Id EV_Toolbar_Label::getToolbarId(void) const
{
	return m_id;
}

const char * EV_Toolbar_Label::getToolbarLabel(void) const
{
	return m_szToolbarLabel;
}

const char * EV_Toolbar_Label::getIconName(void) const
{
	return m_szIconName;
}

const char * EV_Toolbar_Label::getToolTip(void) const
{
	return m_szToolTip;
}

const char * EV_Toolbar_Label::getStatusMsg(void) const
{
	return m_szStatusMsg;
}

/*****************************************************************/

EV_Toolbar_LabelSet::EV_Toolbar_LabelSet(const char * szLanguage,
										 XAP_Toolbar_Id first, XAP_Toolbar_Id last)
{
	// TODO tis bad to call malloc/calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	UT_cloneString(m_szLanguage,szLanguage);
	m_labelTable = (EV_Toolbar_Label **)calloc((last-first+1),sizeof(EV_Toolbar_Label *));
	m_first = first;
	m_last = last;
}

EV_Toolbar_LabelSet::~EV_Toolbar_LabelSet(void)
{
	FREEP(m_szLanguage);

	if (!m_labelTable)
		return;

	UT_uint32 k, kLimit;
	for (k=0, kLimit=(m_last-m_first+1); (k<kLimit); k++)
	  DELETEP(m_labelTable[k]);
	FREEP(m_labelTable);
}

bool EV_Toolbar_LabelSet::setLabel(XAP_Toolbar_Id id,
									  const char * szToolbarLabel,
									  const char * szIconName,
									  const char * szToolTip,
									  const char * szStatusMsg)
{
	if ((id < m_first) || (id > m_last))
		return false;

	UT_uint32 index = (id - m_first);
	DELETEP(m_labelTable[index]);
	m_labelTable[index] = new EV_Toolbar_Label(id,szToolbarLabel,szIconName,szToolTip,szStatusMsg);
	return (m_labelTable[index] != NULL);
}

EV_Toolbar_Label * EV_Toolbar_LabelSet::getLabel(XAP_Toolbar_Id id)
{
	if ((id < m_first) || (id > m_last))
		return NULL;

	UT_uint32 index = (id - m_first);
	
	EV_Toolbar_Label * pLabel = m_labelTable[index];

#if 0
	// IDEA: some labelsets are sparse because their translation is behind
	// HACK: if no label, create a fallback JIT so we don't fail downstream
	// TODO: fall back to English instead like strings do (but not here)
	if (!pLabel)
	{
		UT_ASSERT(
		UT_DEBUGMSG(("WARNING: %s translation for Toolbar id [%d] not found.\n",m_szLanguage,id));
		// NOTE: only translators should see the following strings
		// NOTE: do *not* translate them
		pLabel = new EV_Toolbar_Label(id,"TODO","tb_todo_xpm","to do","untranslated toolbar item");
		m_labelTable[index] = pLabel;
	}
#endif
	UT_ASSERT(pLabel && (pLabel->getToolbarId()==id));
	return pLabel;
}

const char * EV_Toolbar_LabelSet::getLanguage(void) const
{
	return m_szLanguage;
}

void EV_Toolbar_LabelSet::setLanguage(const char *szLanguage)
{
	if (m_szLanguage)
		FREEP(m_szLanguage);
	UT_cloneString(m_szLanguage,szLanguage);
}
