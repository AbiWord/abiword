/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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


#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Actions.h"
#include "xap_Toolbar_Layouts.h"
#include "xap_Prefs.h"
#include "xap_Prefs_SchemeIds.h"
#include "ap_Toolbar_Id.h"

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load the layout for each
** toolbar layout in the application.  It is important that all of
** the ...Layout_*.h files allow themselves to be included more
** than one time.
******************************************************************
*****************************************************************/

#define BeginLayout(Name)		static struct XAP_Toolbar_Factory_lt s_ltTable_##Name[] = {
#define ToolbarItem(id)			{ EV_TLF_Normal,		(id)					},
#define Spacer()				{ EV_TLF_Spacer,		AP_TOOLBAR_ID__BOGUS1__	},
#define EndLayout()				};

#include "ap_Toolbar_Layouts_All.h"

#undef BeginLayout
#undef ToolbarItem
#undef Spacer
#undef EndLayout

/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table containing
** the names and addresses of all the tables we constructed in the
** previous section.
******************************************************************
*****************************************************************/

#define BeginLayout(Name)		{ #Name, NrElements(s_ltTable_##Name), s_ltTable_##Name },
#define ToolbarItem(id)			/*nothing*/
#define Spacer()				/*nothing*/
#define EndLayout()				/*nothing*/

static struct XAP_Toolbar_Factory_tt s_ttTable[] =
{

#include "ap_Toolbar_Layouts_All.h"
	
};

#undef BeginLayout
#undef ToolbarItem
#undef Spacer
#undef EndLayout

/*****************************************************************
******************************************************************
** Put it all together and have a "load Layout by Name"
******************************************************************
*****************************************************************/


XAP_Toolbar_Factory_vec::XAP_Toolbar_Factory_vec(const char * szName)
{
	m_name = szName;
	m_Vec_lt.clear();
}

XAP_Toolbar_Factory_vec::XAP_Toolbar_Factory_vec(XAP_Toolbar_Factory_tt * orig)
{
	m_name = orig->m_name;
	UT_uint32 i = 0;
	m_Vec_lt.clear();
	for(i=0; i < orig->m_nrEntries; i++)
	{
		XAP_Toolbar_Factory_lt * plt = new XAP_Toolbar_Factory_lt;
		plt->m_flags = orig->m_lt[i].m_flags;
		plt->m_id = orig->m_lt[i].m_id;
		m_Vec_lt.addItem((void *) plt);
	}
}

XAP_Toolbar_Factory_vec::XAP_Toolbar_Factory_vec(EV_Toolbar_Layout * orig)
{
	m_name = orig->getName();
	UT_uint32 i = 0;
	m_Vec_lt.clear();
	for(i=0; i < orig->getLayoutItemCount(); i++)
	{
		XAP_Toolbar_Factory_lt * plt = new XAP_Toolbar_Factory_lt;
		plt->m_flags = orig->getLayoutItem(i)->getToolbarLayoutFlags();
		plt->m_id = orig->getLayoutItem(i)->getToolbarId();
		m_Vec_lt.addItem((void *) plt);
	}
}

XAP_Toolbar_Factory_vec::~XAP_Toolbar_Factory_vec(void)
{
	UT_VECTOR_PURGEALL(XAP_Toolbar_Factory_lt *,m_Vec_lt);
}


UT_uint32 XAP_Toolbar_Factory_vec::getNrEntries(void)
{
	return m_Vec_lt.getItemCount();
}

/*!
 * Used to manually construct a XAP_Toolbar_Factory_vec from a preference
 * scheme
 */
void XAP_Toolbar_Factory_vec::add_lt(XAP_Toolbar_Factory_lt * plt)
{
	m_Vec_lt.addItem((void *) plt);
}

XAP_Toolbar_Factory_lt * XAP_Toolbar_Factory_vec::getNth_lt(UT_uint32 i)
{
	XAP_Toolbar_Factory_lt * plt = (XAP_Toolbar_Factory_lt *) m_Vec_lt.getNthItem(i);
	return plt;
}


void XAP_Toolbar_Factory_vec::insertItemBefore(void * p, XAP_Toolbar_Id id)
{
	UT_uint32 i = 0;
	bool bFound = false;
	for(i=0; !bFound && (i< m_Vec_lt.getItemCount()); i++)
	{
		XAP_Toolbar_Factory_lt * plt = (XAP_Toolbar_Factory_lt *) m_Vec_lt.getNthItem(i);
		if(plt->m_id == id)
		{
			m_Vec_lt.insertItemAt(p,i);
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
}


void XAP_Toolbar_Factory_vec::insertItemAfter(void * p, XAP_Toolbar_Id id)
{
	UT_uint32 i = 0;
	bool bFound = false;
	for(i=0; !bFound && (i< m_Vec_lt.getItemCount()); i++)
	{
		XAP_Toolbar_Factory_lt * plt = (XAP_Toolbar_Factory_lt *) m_Vec_lt.getNthItem(i);
		if(plt->m_id == id)
		{
			if((i+1) == m_Vec_lt.getItemCount())
			{
				m_Vec_lt.addItem(p);
			}
			else
			{
				m_Vec_lt.insertItemAt(p,i+1);
			}
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
}

void XAP_Toolbar_Factory_vec::insertLastItem(void * p)
{
	m_Vec_lt.addItem(p);
}

bool XAP_Toolbar_Factory_vec::removeToolbarId(XAP_Toolbar_Id id)
{
	UT_uint32 i = 0;
	bool bFound = false;
	for(i=0; !bFound && (i< m_Vec_lt.getItemCount()); i++)
	{
		XAP_Toolbar_Factory_lt * plt = (XAP_Toolbar_Factory_lt *) m_Vec_lt.getNthItem(i);
		if(plt->m_id == id)
		{
			m_Vec_lt.deleteNthItem(i);
			bFound = true;
			DELETEP(plt);
			break;
		}
	}
	UT_ASSERT(bFound);
	return true;
}

const char * XAP_Toolbar_Factory_vec::getToolbarName(void)
{
	return m_name.c_str();
}


/***************************************************************************/

XAP_Toolbar_Factory::XAP_Toolbar_Factory(XAP_App * pApp)
{
	m_pApp = pApp;
	UT_uint32 i = 0;
	UT_uint32 count = NrElements(s_ttTable);
//
// FIXME: Put in check on preference
//
	for(i=0; i < count; i++)
	{
		XAP_Toolbar_Factory_vec * pVec = new XAP_Toolbar_Factory_vec(&s_ttTable[i]);
		m_vecTT.addItem((void *) pVec);
	}
}


XAP_Toolbar_Factory::~XAP_Toolbar_Factory(void)
{
	UT_VECTOR_PURGEALL(XAP_Toolbar_Factory_vec *,m_vecTT);
}


EV_Toolbar_Layout * XAP_Toolbar_Factory::CreateToolbarLayout(const char * szName)
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	EV_Toolbar_Layout * pLayout = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		XAP_Toolbar_Factory_vec * pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		if (UT_stricmp(szName,szCurName)==0)
		{
			bFound = true;
			pLayout = new EV_Toolbar_Layout(pVec->getToolbarName(),pVec->getNrEntries());
			
			UT_ASSERT(pLayout);
			UT_uint32 k = 0;
			for (k=0; k < pVec->getNrEntries(); k++)
			{
				XAP_Toolbar_Factory_lt * plt = pVec->getNth_lt(k);
				bool bResult = pLayout->setLayoutItem(k, plt->m_id, plt->m_flags);
				UT_ASSERT(bResult);
			}
			break;
		}
	}
	UT_ASSERT(bFound);
	if(bFound)
	{
		return pLayout;
	}
	return NULL;
}

EV_Toolbar_Layout * XAP_Toolbar_Factory::DuplicateToolbarLayout(const char * szName)
{
	return CreateToolbarLayout(szName);
}

/*!
 * This restores a toolbar layout in memory to a version previously constructed
 * and used as Toolbar.
 */
void XAP_Toolbar_Factory::restoreToolbarLayout(EV_Toolbar_Layout *pTB)
{
	UT_ASSERT(pTB);
	UT_String strName = pTB->getName();
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		if (UT_stricmp(strName.c_str(),szCurName)==0)
		{
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
	DELETEP(pVec);
	pVec = new XAP_Toolbar_Factory_vec(pTB);
	m_vecTT.setNthItem(i, (void *) pVec, NULL);
}


/*!
 * This method inserts an icon before the icon number beforeId
 */			
bool  XAP_Toolbar_Factory::addIconBefore(const char * szName,
								   XAP_Toolbar_Id newId, 
								   XAP_Toolbar_Id beforeId)
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		if (UT_stricmp(szName,szCurName)==0)
		{
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
	if(!bFound)
	{
		return false;
	}
	XAP_Toolbar_Factory_lt * plt = new  XAP_Toolbar_Factory_lt;
	plt->m_id = newId;
	plt->m_flags = EV_TLF_Normal;
	pVec->insertItemBefore((void *) plt, beforeId);
	return true;
}

/*!
 * This method adds an icon at the last position on a toolbar.
 */			
bool  XAP_Toolbar_Factory::addIconAtEnd(const char * szName,
								   XAP_Toolbar_Id newId )
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		if (UT_stricmp(szName,szCurName)==0)
		{
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
	if(!bFound)
	{
		return false;
	}
	XAP_Toolbar_Factory_lt * plt = new  XAP_Toolbar_Factory_lt;
	plt->m_id = newId;
	plt->m_flags = EV_TLF_Normal;
	pVec->insertLastItem((void *) plt);
	return true;
}

			
bool  XAP_Toolbar_Factory::addIconAfter(const char * szName,
								   XAP_Toolbar_Id newId, 
								   XAP_Toolbar_Id afterId)
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		if (UT_stricmp(szName,szCurName)==0)
		{
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
	if(!bFound)
	{
		return false;
	}
	XAP_Toolbar_Factory_lt * plt = new  XAP_Toolbar_Factory_lt;
	plt->m_id = newId;
	plt->m_flags = EV_TLF_Normal;
	pVec->insertItemAfter((void *) plt, afterId);
	return true;
}

			
bool  XAP_Toolbar_Factory::removeIcon(const char * szName,
									  XAP_Toolbar_Id nukeId)
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		if (UT_stricmp(szName,szCurName)==0)
		{
			bFound = true;
			break;
		}
	}
	UT_ASSERT(bFound);
	if(!bFound)
	{
		return false;
	}
	pVec->removeToolbarId(nukeId);
	return true;
}

bool XAP_Toolbar_Factory::resetAllToolbarsToDefault(void)
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; i < count; i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		const char * szCurName =  pVec->getToolbarName();
		resetToolbarToDefault(szCurName);
	}
	return true;
}
			
bool  XAP_Toolbar_Factory::resetToolbarToDefault(const char * szName)
{
	UT_uint32 count = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 i = 0;
	bool bFound = false;
	const char * szCurName = NULL;
	XAP_Toolbar_Factory_vec * pVec = NULL;
	for (i=0; !bFound && (i < count); i++)
	{
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(i);
		szCurName =  UT_strdup(pVec->getToolbarName());
		if (UT_stricmp(szName,szCurName)==0)
		{
			bFound = true;
			break;
		}
		else
		{
			FREEP( szCurName );
		}
	}
	UT_ASSERT(bFound);
	if(!bFound)
	{
		return false;
	}
	DELETEP(pVec);
	UT_uint32 j = 0;
	count = NrElements(s_ttTable);
	bFound = false;
	for(j=0; !bFound &&( j < count); j++)
	{
		if(UT_stricmp(szCurName,s_ttTable[j].m_name)== 0)
		{
			bFound = true;
			break;
		}
	}
	FREEP(szCurName);
	UT_ASSERT(bFound);
	if(!bFound)
	{
		return false;
	}
	pVec = new XAP_Toolbar_Factory_vec(&s_ttTable[j]);
	m_vecTT.setNthItem(i, (void *) pVec, NULL);
	return true;
}

/*!
 * This method saves the current toolbar layouts in the current preference
 * scheme. There are 3 generic keys.
 * The Number of entries ((Base Key for Nr entries)+(TB name))
 * The ith ID ((Base Key for id)+(TB name)+(i))
 * The ith Flag ((Base Key for flag)+(TB name)+(i))
 */
bool  XAP_Toolbar_Factory::saveToolbarsInCurrentScheme(void)
{
	XAP_Prefs *pPrefs = m_pApp->getPrefs();
	XAP_PrefsScheme *pScheme = pPrefs->getCurrentScheme(true);
	char buf[100];
	XAP_Toolbar_Factory_vec * pVec = NULL;
	UT_uint32 numTB = m_vecTT.getItemCount();  // NO toolabrs
	UT_uint32 iTB,iLay;
	for(iTB=0; iTB< numTB;iTB++)
	{
		UT_String sTBBase = XAP_PREF_KEY_ToolbarNumEntries;
		pVec = (XAP_Toolbar_Factory_vec *) m_vecTT.getNthItem(iTB);
		const char * szCurName =  pVec->getToolbarName();
//
// Save Number of entries in contructed key
//
		sTBBase +=szCurName;
		UT_uint32 NrEntries = pVec->getNrEntries();
		UT_DEBUGMSG(("SEVIOR: Number of entries in TB %d \n",NrEntries));
		sprintf(buf,"%d",NrEntries);
		pScheme->setValue((const XML_Char *)sTBBase.c_str(),(const XML_Char *) buf );
//
// Loop through this toolbar definition and save it in the preferences
//		
		for(iLay =0; iLay< NrEntries;iLay++)
		{
			XAP_Toolbar_Factory_lt * plt = pVec->getNth_lt(iLay);
			sTBBase = XAP_PREF_KEY_ToolbarID;
			sTBBase += szCurName;
//
// Save id in contructed key
//
			XAP_Toolbar_Id curId = plt->m_id;
			EV_Toolbar_LayoutFlags curFlag = plt->m_flags;
			sprintf(buf,"%d",iLay);
			sTBBase += buf;
			sprintf(buf,"%d",curId);
			pScheme->setValue((const XML_Char *) sTBBase.c_str(),(const XML_Char *) buf );
//
// Save flags in contructed key
//
			sTBBase = XAP_PREF_KEY_ToolbarFlag;
			sTBBase += szCurName;
			sprintf(buf,"%d",iLay);
			sTBBase += buf;
			sprintf(buf,"%d",curFlag);
			pScheme->setValue((const XML_Char *) sTBBase.c_str(),(const XML_Char *) buf );
		}
	}
	return true;
}


/*!
 * This method restores the toolbar layouts from the current preference
 * scheme. There are 3 generic keys.
 * The Number of entries ((Base Key for Nr entries)+(TB name))
 * The ith ID ((Base Key for id)+(TB name)+(i))
 * The ith Flag ((Base Key for flag)+(TB name)+(i))
 */
bool  XAP_Toolbar_Factory::restoreToolbarsFromCurrentScheme(void)
{
//
// First delete the current layouts.
//
	UT_VECTOR_PURGEALL(XAP_Toolbar_Factory_vec *,m_vecTT);
	m_vecTT.clear();
//
// Get the current scheme
//
	XAP_Prefs *pPrefs = m_pApp->getPrefs();
	XAP_PrefsScheme *pScheme = pPrefs->getCurrentScheme(true);
	char buf[100];
	XAP_Toolbar_Factory_vec * pVec = NULL;
//
// Get number of toolbars hardwired into abiword.
//
	UT_uint32 numTB = NrElements(s_ttTable);  // NO toolabrs
	UT_uint32 iTB,iLay;
	for(iTB=0; iTB< numTB;iTB++)
	{
		UT_String sTBBase = XAP_PREF_KEY_ToolbarNumEntries;
		const char * szCurName =  s_ttTable[iTB].m_name;
		sTBBase +=szCurName;
		const XML_Char * szNrEntries = NULL;
		UT_uint32 NrEntries = 0;
//
// Get Number of entries if the correct key exists. Otherwise use defaults.
//
		pScheme->getValue((const XML_Char *)sTBBase.c_str(),&szNrEntries);
		if(szNrEntries && *szNrEntries)
		{	
			pVec = new XAP_Toolbar_Factory_vec(szCurName);
			m_vecTT.addItem((void *) pVec);
			NrEntries = atoi(szNrEntries);
//
// Loop through this toolbar definition and restore it from the preferences
//		
			for(iLay =0; iLay< NrEntries;iLay++)
			{
//
// Restore id from the contructed key
//
				sTBBase = XAP_PREF_KEY_ToolbarID;
				sTBBase += szCurName;
				sprintf(buf,"%d",iLay);
				sTBBase += buf;
				const XML_Char * szCurId = NULL;
				pScheme->getValue((const XML_Char *)sTBBase.c_str(),&szCurId);
				UT_ASSERT(szCurId && *szCurId);
				XAP_Toolbar_Id curId = (XAP_Toolbar_Id) atoi(szCurId);
//
// Here we should check whether the ID exists or not
// 
				EV_Toolbar_Action * pAction = m_pApp->getToolbarActionSet()->getAction(curId);
				if (pAction == NULL) {
					UT_DEBUGMSG (("Found an unknown toolbar item in prefs. Ignoring.\n"));
					continue;
				}
//
// Restore flags from the constructed key
//
				sTBBase = XAP_PREF_KEY_ToolbarFlag;
				sTBBase += szCurName;
				sprintf(buf,"%d",iLay);
				sTBBase += buf;
				const XML_Char * szCurFlag = NULL;
				pScheme->getValue((const XML_Char *)sTBBase.c_str(),&szCurFlag);
				UT_ASSERT(szCurFlag && *szCurFlag);
				EV_Toolbar_LayoutFlags curFlag = (EV_Toolbar_LayoutFlags) atoi(szCurFlag);
//
// Build element and add it into the Toolbar layout
//
				XAP_Toolbar_Factory_lt * plt = new XAP_Toolbar_Factory_lt;
				plt->m_id = curId;
				plt->m_flags = curFlag;
				pVec->add_lt(plt);
			}
		}
//
// Use default hardwired layout
//
		else
		{
			pVec = new XAP_Toolbar_Factory_vec(&s_ttTable[iTB]);
			m_vecTT.addItem((void *) pVec);
		}
	}
	return true;
}



