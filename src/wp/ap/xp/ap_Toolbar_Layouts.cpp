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
#include "xap_Toolbar_Layouts.h"
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

			
bool  XAP_Toolbar_Factory::resetToolbarToDefault(const char * szName)
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
	DELETEP(pVec);
	UT_uint32 j = 0;
	count = NrElements(s_ttTable);
	bFound = false;
	for(j=0; !bFound &&( j < count); j++)
	{
		if(UT_stricmp(szName,s_ttTable[j].m_name)== 0)
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
	pVec = new XAP_Toolbar_Factory_vec(&s_ttTable[j]);
	m_vecTT.setNthItem(i, (void *) pVec, NULL);
	return true;
}






