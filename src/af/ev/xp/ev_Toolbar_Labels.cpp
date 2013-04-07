/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * BIDI Copyright (C) 2001,2002 Tomas Frydrych
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_Toolbar_Labels.h"
#include "ut_wctomb.h"
#include "ut_mbtowc.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"

/*****************************************************************/

EV_Toolbar_Label::EV_Toolbar_Label(XAP_Toolbar_Id id,
								   const char * szToolbarLabel,
								   const char * szIconName,
								   const char * szToolTip,
								   const char * szStatusMsg)
{

	m_id = id;
	m_szToolbarLabel = g_strdup(szToolbarLabel);
	m_szIconName = g_strdup(szIconName);
	m_szToolTip = g_strdup(szToolTip);
	m_szStatusMsg = g_strdup(szStatusMsg);

	// TODO: the following code causes crashes; after spending several hours over it
	// it would appear that the glib memory functions that fribidi uses
	// clash with the memory management used by new; consequently the two
	// temp. FribidiChar strings we allocate overlap with the memory chunk that
	// fribidi allocates for creating type links, and when we copy our string
	// to the temp we overwrite the runlist
	// I do not know how to fix this and as we do not need it yet,
	// I am going to turn it off for the moment

	// TODO I wish we did not have to do this here, but I see no other
	// way; the menu mechanism is much cleaner and I think we should be
	// using the string-set mechanism for toolbars as we do for menus.
	// when/if we do, this whole bit can be removed
	if(XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_NONE)
	{
        UT_uint32 iOldLen = 0;
        UT_UCS4Char *fbdStr = 0, *fbdStr2 = 0;

		const char * encoding = (XAP_EncodingManager::get_instance()->getNativeSystemEncodingName()) ?
		  XAP_EncodingManager::get_instance()->getNativeSystemEncodingName() :
		  XAP_EncodingManager::get_instance()->getNativeEncodingName();

		UT_UCS4_mbtowc mbtowc_conv(encoding);
		UT_Wctomb wctomb_conv(encoding);
		UT_UCS4Char wc;

		char letter_buf[20];
		int length;

        char * szStr = m_szToolTip;

        for(UT_uint32 l = 0; l < 2; l++)
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

					fbdStr   = new UT_UCS4Char [iStrLen + 1];
					UT_return_if_fail(fbdStr);
					fbdStr2  = new UT_UCS4Char [iStrLen + 1];
					UT_return_if_fail(fbdStr2);
					iOldLen = iStrLen;
				}

				UT_uint32 i;
				UT_uint32 j = 0;
				UT_uint32 k = 0;
				for(i = 0; i < iStrLen; i++)
				{
					if(mbtowc_conv.mbtowc(wc,szStr[i]))
					{
						fbdStr[j++] = wc;
					}
				}

				// TODO -- this should be lang based as we do for string set
				UT_BidiCharType iDomDir = UT_bidiGetCharType(fbdStr[0]);

// this has been crashing with en-US (but not en-GB), due to some
// weird memory managment in the fribidi library; so I defined
// USE_SIMPLE_MALLOC for it, which solved the problem
				UT_bidiReorderString(fbdStr, j, iDomDir, fbdStr2);

				for(i = 0; i < j; i++)
				{
					if (wctomb_conv.wctomb(letter_buf,length,fbdStr2[i]))
					{
						for(k = 0; k < static_cast<unsigned>(length); k++)
							szStr[i++] = letter_buf[k];
						i--;
					}
				}

				UT_ASSERT(szStr[i] == 0);
			}

			szStr = m_szStatusMsg;
		}

		delete[] fbdStr;
		delete[] fbdStr2;
	}
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
	// TODO tis bad to call g_try_malloc/UT_calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	m_szLanguage = g_strdup(szLanguage);
	m_labelTable = static_cast<EV_Toolbar_Label **>(UT_calloc((last-first+1),sizeof(EV_Toolbar_Label *)));
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
	m_szLanguage = g_strdup(szLanguage);
}
