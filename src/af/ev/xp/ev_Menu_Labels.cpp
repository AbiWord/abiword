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
#include "ev_Menu_Labels.h"
#include "ut_vector.h"
#ifdef BIDI_ENABLED
#include "fribidi.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"
#endif

/*****************************************************************/

EV_Menu_Label::EV_Menu_Label(XAP_Menu_Id id,
							 const char * szMenuLabel,
							 const char * szStatusMsg)
	: m_id(id)
#ifndef BIDI_ENABLED
,
	  m_stMenuLabel(szMenuLabel),
	  m_stStatusMsg(szStatusMsg)
#endif
{
#ifdef BIDI_ENABLED

	if(!XAP_App::getApp()->theOSHasBidiSupport())
	{
		UT_uint32 iLabelLen  = strlen(szMenuLabel);
		UT_uint32 iStatusLen = strlen(szStatusMsg);
	
		FriBidiChar * fbdLabel   = new FriBidiChar [iLabelLen + 1];
		UT_ASSERT(fbdLabel);
		FriBidiChar * fbdLabel2  = new FriBidiChar [iLabelLen + 1];
		UT_ASSERT(fbdLabel2);
		char * szMenuLabel2 = new char [iLabelLen + 1];
		UT_ASSERT(szMenuLabel2);
	
		FriBidiChar * fbdStatus  = new FriBidiChar [iStatusLen + 1];
		UT_ASSERT(fbdStatus);
		FriBidiChar * fbdStatus2 = new FriBidiChar [iStatusLen + 1];
		UT_ASSERT(fbdStatus2);
		char * szStatusMsg2 = new char [iStatusLen + 1];
		UT_ASSERT(szStatusMsg2);
	
		UT_uint32 i;
		for(i = 0; i < iLabelLen; i++)
			fbdLabel[i] = (FriBidiChar) XAP_EncodingManager::get_instance()->nativeToU((UT_UCSChar)szMenuLabel[i]);
		fbdLabel[i] = 0;
	
		for(i = 0; i < iStatusLen; i++)
			fbdStatus[i] = (FriBidiChar) XAP_EncodingManager::get_instance()->nativeToU((UT_UCSChar)szStatusMsg[i]);
		fbdStatus[i] = 0;

	
		FriBidiCharType fbdDomDir = fribidi_get_type(fbdLabel[0]);
	
		fribidi_log2vis (		/* input */
		       fbdLabel,
		       iLabelLen,
		       &fbdDomDir,
		       /* output */
		       fbdLabel2,
		       NULL,
		       NULL,
		       NULL);	

		fbdDomDir = fribidi_get_type(fbdStatus[0]);
			
		fribidi_log2vis (		/* input */
		       fbdStatus,
		       iStatusLen,
		       &fbdDomDir,
		       /* output */
		       fbdStatus2,
		       NULL,
		       NULL,
		       NULL);
		
	
		for(i = 0; i < iLabelLen; i++)
			szMenuLabel2[i] = (char) XAP_EncodingManager::get_instance()->UToNative((UT_UCSChar)fbdLabel2[i]);
		szMenuLabel2[i] = 0;
	
		for(i = 0; i < iStatusLen; i++)
			szStatusMsg2[i] = (char) XAP_EncodingManager::get_instance()->UToNative((UT_UCSChar)fbdStatus2[i]);
		szStatusMsg2[i] = 0;

		m_stMenuLabel = szMenuLabel2;
		m_stStatusMsg = szStatusMsg2;
		
		delete[] fbdLabel;
		delete[] fbdLabel2;
		delete[] fbdStatus;
		delete[] fbdStatus2;
		delete[] szMenuLabel2;
		delete[] szStatusMsg2;
	}
	else
	{
		m_stMenuLabel = szMenuLabel;
		m_stStatusMsg = szStatusMsg;
	}
	
#endif
}

EV_Menu_Label::~EV_Menu_Label()
{
}

XAP_Menu_Id
EV_Menu_Label::getMenuId() const
{
	return m_id;
}

const char *
EV_Menu_Label::getMenuLabel() const
{
	return m_stMenuLabel.c_str();
}

const char *
EV_Menu_Label::getMenuStatusMessage() const
{
	return m_stStatusMsg.c_str();
}

/*****************************************************************/

EV_Menu_LabelSet::EV_Menu_LabelSet(const char * szLanguage,
								   XAP_Menu_Id first, XAP_Menu_Id last)
	: m_labelTable(last - first + 1),
	  m_first(first),
	  m_stLanguage(szLanguage)
{
	size_t size = last - first + 1;
	
	for (size_t i = 0; i < size; ++i)
		m_labelTable.addItem(0);
}


EV_Menu_LabelSet::EV_Menu_LabelSet(EV_Menu_LabelSet * pLabelSet)
{
	m_stLanguage = pLabelSet->getLanguage();
	m_first = pLabelSet->getFirst();
	const UT_Vector * vecLabels = pLabelSet->getAllLabels();
	UT_uint32 i = 0;
	for(i=0; i< vecLabels->getItemCount(); i++)
	{
	    EV_Menu_Label * pEvl = (EV_Menu_Label *) vecLabels->getNthItem(i);
		EV_Menu_Label * pNewLab = NULL;
		if(pEvl != NULL)
		{
		    pNewLab = new EV_Menu_Label(pEvl->getMenuId(),pEvl->getMenuLabel(),pEvl->getMenuStatusMessage());
		}
		m_labelTable.addItem( (void *) pNewLab);
	}
}

EV_Menu_LabelSet::~EV_Menu_LabelSet()
{
	UT_VECTOR_SPARSEPURGEALL(EV_Menu_Label *, m_labelTable);
}

bool EV_Menu_LabelSet::setLabel(XAP_Menu_Id id,
								const char * szMenuLabel,
								const char * szStatusMsg)
{
	void *tmp;
	XAP_Menu_Id last = m_first + m_labelTable.size();
	
	if (id < m_first || id >= last)
		return false;

	UT_sint32 index = id - m_first;
	EV_Menu_Label *label = new EV_Menu_Label(id, szMenuLabel, szStatusMsg);
	UT_sint32 error = m_labelTable.setNthItem(index, label, &tmp);
	EV_Menu_Label * pTmpLbl = static_cast<EV_Menu_Label *> (tmp);
	DELETEP(pTmpLbl);
	return (error == 0);
}

#ifdef __MRC__
EV_Menu_Label * EV_Menu_LabelSet::getLabel(XAP_Menu_Id id)
#else
EV_Menu_Label * EV_Menu_LabelSet::getLabel(XAP_Menu_Id id) const
#endif
{
	XAP_Menu_Id last = m_first + m_labelTable.size();
	if (id < m_first || id >= last)
		return NULL;

	UT_uint32 index = (id - m_first);
	
	EV_Menu_Label * pLabel = static_cast<EV_Menu_Label *> (m_labelTable.getNthItem(index));

	if (!pLabel)
	{
		UT_DEBUGMSG(("WARNING: %s translation for menu id [%d] not found.\n", m_stLanguage.c_str(), id));
		// NOTE: only translators should see the following strings
		// NOTE: do *not* translate them
		pLabel = new EV_Menu_Label(id, "TODO", "untranslated menu item");
		
		// Add to label table so memory is freed.
		// Note: Need to cast away constness so we can add the label.
		((EV_Menu_LabelSet *)this)->addLabel(pLabel);
	}

	UT_ASSERT(pLabel && (pLabel->getMenuId() == id));
	return pLabel;
}

bool EV_Menu_LabelSet::addLabel(EV_Menu_Label *pLabel)
{
	UT_DEBUGMSG(("JCA: EV_Menu_LabelSet::addLabel\n"));
	UT_ASSERT(pLabel);
	XAP_Menu_Id size_table = m_labelTable.size();

	// the if (...) is here due to
	// AP_MENU_ID__BOGUS2__, which ocupes an entry in the
	// action table, but that not in the layout table
	// the real fix is to erase AP_MENU_ID__BOGUS2__
	if (pLabel->getMenuId() == size_table + m_first - 1)
	{
		m_labelTable.pop_back();
		size_table = m_labelTable.size();
	}
	
	UT_ASSERT(pLabel->getMenuId() == size_table + m_first);
	m_labelTable.push_back(pLabel);

	return (size_table + 1 == (XAP_Menu_Id) m_labelTable.size());
}

const char * EV_Menu_LabelSet::getLanguage() const
{
	return m_stLanguage.c_str();
}

void EV_Menu_LabelSet::setLanguage(const char *szLanguage)
{
	m_stLanguage = szLanguage;
}

