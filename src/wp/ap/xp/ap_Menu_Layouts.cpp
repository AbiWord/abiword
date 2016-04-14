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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "ev_Menu_Labels.h"
#include "ev_Menu.h"

#include "xap_App.h"
#include "xap_Menu_Layouts.h"
#include "xap_Menu_ActionSet.h"
#include "xap_EncodingManager.h"

#include "ap_Menu_Id.h"
#include "ap_Strings.h"

	struct _lt
	{
		EV_Menu_LayoutFlags			m_flags;
		XAP_Menu_Id					m_id;
	};

	struct _tt
	{
		const char *				m_name;
		UT_uint32					m_nrEntries;
		struct _lt *				m_lt;
		EV_EditMouseContext			m_emc;
	};

	class ABI_EXPORT _vectt
	{
	public:
		_vectt(_tt * orig):
			m_Vec_lt(orig->m_nrEntries, 4, true)
			{
				m_name = orig->m_name;
		        m_emc = orig->m_emc;
				m_Vec_lt.clear();
				UT_uint32 k = 0;
				for(k = 0; k < orig->m_nrEntries; k++)
				{
					_lt * plt = new _lt;
					*plt = orig->m_lt[k];
					m_Vec_lt.addItem(plt);
				}
			};
		~_vectt()
			{
				UT_VECTOR_PURGEALL(_lt *,m_Vec_lt);
			};
		UT_uint32 getNrEntries(void)
			{
				return m_Vec_lt.getItemCount();
			};
		_lt * getNth_lt(UT_uint32 n)
			{
				return m_Vec_lt.getNthItem(n);
			};
		void insertItemAt(_lt * p, XAP_Menu_Id id)
			{
				UT_sint32 i = 0;
				bool bFound = false;
				for(i=0; i< m_Vec_lt.getItemCount() && !bFound; i++)
				{
					_lt * plt = m_Vec_lt.getNthItem(i);
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
					}
				}
				UT_ASSERT_HARMLESS(bFound);
			};
		void insertItemBefore(_lt * p, XAP_Menu_Id id)
			{
				UT_sint32 i = 0;
				bool bFound = false;
				for(i=0; i< m_Vec_lt.getItemCount() && !bFound; i++)
				{
					_lt * plt = m_Vec_lt.getNthItem(i);
					if(plt->m_id == id)
					{
						if((i+1) == m_Vec_lt.getItemCount())
						{
							m_Vec_lt.addItem(p);
						}
						else
						{
							m_Vec_lt.insertItemAt(p,i);
						}
						bFound = true;
					}
				}
				UT_ASSERT_HARMLESS(bFound);
			};
		void removeItem(XAP_Menu_Id id)
			{
				UT_sint32 i = 0;
				bool bFound = false;
				for(i=0; i< m_Vec_lt.getItemCount() && !bFound; i++)
				{
					_lt * plt = m_Vec_lt.getNthItem(i);
					if(plt->m_id == id)
					{
						m_Vec_lt.deleteNthItem(i);
						delete plt;
						bFound = true;
					}
				}
				UT_ASSERT_HARMLESS(bFound);
			};
		void setNthIDFlags(UT_uint32 n, XAP_Menu_Id id,EV_Menu_LayoutFlags flags)
			{
				_lt * plt = m_Vec_lt.getNthItem(n);
				plt->m_flags = flags;
				plt->m_id = id;
			};
		const char *				m_name;
		EV_EditMouseContext			m_emc;
	private:
		UT_GenericVector<_lt*>		m_Vec_lt;
	};


/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load the layout for each
** menu layout in the application.  It is important that all of
** the ...Layout_*.h files allow themselves to be included more
** than one time.
******************************************************************
*****************************************************************/

#define BeginLayout(Name,Cxt)	static struct _lt s_ltTable_##Name[] = {
#define MenuItem(id)			{ EV_MLF_Normal,		(id)				 },
#define BeginSubMenu(id)		{ EV_MLF_BeginSubMenu,	(id)				 },
#define BeginPopupMenu()		{ EV_MLF_BeginPopupMenu,AP_MENU_ID__BOGUS1__ },
#define Separator()				{ EV_MLF_Separator,		AP_MENU_ID__BOGUS1__ },
#define EndSubMenu()			{ EV_MLF_EndSubMenu,	AP_MENU_ID__BOGUS1__ },
#define EndPopupMenu()			{ EV_MLF_EndPopupMenu,	AP_MENU_ID__BOGUS1__ },
#define EndLayout()				};

#include "ap_Menu_Layouts_All.h"

#undef BeginLayout
#undef MenuItem
#undef BeginSubMenu
#undef BeginPopupMenu
#undef Separator
#undef EndSubMenu
#undef EndPopupMenu
#undef EndLayout


/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table containing
** the names and addresses of all the tables we constructed in the
** previous section.
******************************************************************
*****************************************************************/

#define BeginLayout(Name,Cxt)	{ #Name, G_N_ELEMENTS(s_ltTable_##Name), s_ltTable_##Name, Cxt },
#define MenuItem(id)			/*nothing*/
#define BeginSubMenu(id)		/*nothing*/
#define BeginPopupMenu()		/*nothing*/
#define Separator()				/*nothing*/
#define EndSubMenu()			/*nothing*/
#define EndPopupMenu()			/*nothing*/
#define EndLayout()				/*nothing*/

static struct _tt s_ttTable[] =
{

#include "ap_Menu_Layouts_All.h"
	
};

#undef BeginLayout
#undef MenuItem
#undef BeginSubMenu
#undef BeginPopupMenu
#undef Separator
#undef EndSubMenu
#undef EndPopupMenu
#undef EndLayout



/*****************************************************************
******************************************************************
** Put it all together and have a "load Layout by Name"
******************************************************************
*****************************************************************/


/*!
 * Load these cleverly constructed static tables into vectors so they can be 
 * manipulated dynamically.
 */
XAP_Menu_Factory::XAP_Menu_Factory(XAP_App * pApp) :
		m_pApp(pApp),
        m_pLabelSet(NULL),
        m_maxID(0)
{
	UT_uint32 k = 0;
	m_vecTT.clear();
	for (k=0; k<G_N_ELEMENTS(s_ttTable); k++)
	{
		_vectt * pVectt = new _vectt(&s_ttTable[k]);
		m_vecTT.addItem(pVectt);
	}
	m_pEnglishLabelSet = NULL;
	m_pBSS = NULL;
	m_NextContext = EV_EMC_AVAIL;
}

XAP_Menu_Factory::~XAP_Menu_Factory()
{
    UT_VECTOR_SPARSEPURGEALL(_vectt *,m_vecTT);
	DELETEP(m_pEnglishLabelSet);
	DELETEP(m_pBSS);
	DELETEP(m_pLabelSet);
}

XAP_Menu_Id XAP_Menu_Factory::getNewID(void)
{
	if(m_maxID > 0)
	{
		m_maxID++;
		return m_maxID;
	}
	UT_sint32 i =0;
	UT_uint32 j =0;
	for(i=0; i < m_vecTT.getItemCount(); i++)
	{
		_vectt * pTT = m_vecTT.getNthItem(i);
		if (pTT == NULL)
			continue;
		for(j=0; j < pTT->getNrEntries(); j++)
		{
			_lt * plt = (_lt *) pTT->getNth_lt(j);
			if(plt->m_id > m_maxID)
			{
				m_maxID = plt->m_id;
			}
		}
	}
	m_maxID++;
	return m_maxID;
}
		

EV_Menu_Layout * XAP_Menu_Factory::CreateMenuLayout(const char * szName)
{
	UT_return_val_if_fail (szName && *szName, NULL);		// no defaults

	for (UT_sint32 k=0; k< m_vecTT.getItemCount(); k++)
	{
		_vectt * pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		if (g_ascii_strcasecmp(szName,pVectt->m_name)==0)
		{
			UT_uint32 NrEntries = pVectt->getNrEntries();
			EV_Menu_Layout * pLayout = new EV_Menu_Layout(pVectt->m_name,NrEntries);
			UT_return_val_if_fail (pLayout, NULL);
			
			for (UT_uint32 j=0; (j < NrEntries); j++)
			{
				_lt * plt = pVectt->getNth_lt(j);
				UT_DebugOnly<bool> bResult = pLayout->setLayoutItem(j, plt->m_id, plt->m_flags);
				UT_ASSERT_HARMLESS(bResult);
			}

			return pLayout;
		}
	}
	UT_ASSERT_HARMLESS(0);						// no defaults
	return NULL;
}

const char * XAP_Menu_Factory::FindContextMenu(EV_EditMouseContext emc)
{

	for (UT_sint32 k=0; k< m_vecTT.getItemCount(); k++)
	{
		_vectt * pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		UT_DEBUGMSG(("Look menu %s id %x requested %x  \n",pVectt->m_name,pVectt->m_emc,emc));
		if (emc == pVectt->m_emc)
		{
			return pVectt->m_name;
		}
	}
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return NULL;
}

XAP_Menu_Id XAP_Menu_Factory::addNewMenuAfter(const char * szMenu, 
											   const char * /*szLanguage*/,
											   const XAP_Menu_Id afterID, 
											   EV_Menu_LayoutFlags flags, XAP_Menu_Id newID)
{
	UT_return_val_if_fail (szMenu && *szMenu, 0);		// no defaults
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		bFoundMenu = (g_ascii_strcasecmp(szMenu,pVectt->m_name)==0);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	
//
// OK got the menu ID at last, insert the new id here.
//
	if(newID == 0)
	{
		newID = getNewID();
	}
//
// Now insert our new ID
//
	_lt * plt = new _lt;
	plt->m_id = newID;
	plt->m_flags = flags;
	pVectt->insertItemAt(plt, afterID);
	return (XAP_Menu_Id) newID;
}

XAP_Menu_Id XAP_Menu_Factory::addNewMenuAfter(const char * szMenu, 
											  const char * /*szLanguage*/,
											  const char * szAfter, 
											  EV_Menu_LayoutFlags flags,
											  XAP_Menu_Id newID)
{
	UT_return_val_if_fail (szMenu && *szMenu, 0);		// no defaults
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		bFoundMenu = (g_ascii_strcasecmp(szMenu,pVectt->m_name)==0);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	
// next we need to find the id of the label name
//
// OK now search for this label
//
	UT_String After = szAfter;
	XAP_Menu_Id afterID = EV_searchMenuLabel( m_pLabelSet, After);
	if(afterID == 0)
	{
		if(m_pEnglishLabelSet == NULL)
		{
			buildBuiltInMenuLabelSet( m_pEnglishLabelSet);
		}
		afterID = EV_searchMenuLabel( m_pEnglishLabelSet, After);
		if(afterID == 0)
		{	
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return 0;
		}
	}
//
// OK got the menu ID at last, insert the new id here.
//
	if(newID == 0)
	{
		newID = getNewID();
	}
//
// Now insert our new ID
//
	_lt * plt = new _lt;
	plt->m_id = newID;
	plt->m_flags = flags;
	pVectt->insertItemAt(plt, afterID);
	return (XAP_Menu_Id) newID;
}

XAP_Menu_Id XAP_Menu_Factory::addNewMenuBefore(const char * szMenu, 
											   const char * /*szLanguage*/,
											   const XAP_Menu_Id beforeID, 
											   EV_Menu_LayoutFlags flags, XAP_Menu_Id newID)
{
	UT_return_val_if_fail (szMenu && *szMenu, 0);		// no defaults
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		bFoundMenu = (g_ascii_strcasecmp(szMenu,pVectt->m_name)==0);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	
//
// OK got the menu ID at last, insert the new id here.
//
	if(newID == 0)
	{
		newID = getNewID();
	}
//
// Now insert our new ID
//
	_lt * plt = new _lt;
	plt->m_id = newID;
	plt->m_flags = flags;
	if(beforeID > 0)
	{
	  pVectt->insertItemBefore(plt, beforeID);
	}
	else
	{
	  pVectt->insertItemAt(plt, beforeID);
	}
	return (XAP_Menu_Id) newID;
}

XAP_Menu_Id XAP_Menu_Factory::addNewMenuBefore(const char * szMenu, 
											   const char * /*szLanguage*/,
											   const char * szBefore, 
											   EV_Menu_LayoutFlags flags, XAP_Menu_Id newID)
{
	UT_return_val_if_fail (szMenu && *szMenu, 0);		// no defaults
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		bFoundMenu = (g_ascii_strcasecmp(szMenu,pVectt->m_name)==0);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	
	XAP_Menu_Id beforeID;
// next we need to find the id of the label name
//
// OK now search for this label
//
	if (szBefore)
	{
		UT_String Before = szBefore;
		beforeID = EV_searchMenuLabel( m_pLabelSet, Before);
		if(beforeID == 0)
		{
			if(m_pEnglishLabelSet == NULL)
			{
				buildBuiltInMenuLabelSet( m_pEnglishLabelSet);
			}
			beforeID = EV_searchMenuLabel( m_pEnglishLabelSet, Before);
			if(beforeID == 0)
			{	
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				return 0;
			}
		}
	}
	else
		beforeID = 0;
//
// OK got the menu ID at last, insert the new id here.
//
	if(newID == 0)
	{
		newID = getNewID();
	}
//
// Now insert our new ID
//
	_lt * plt = new _lt;
	plt->m_id = newID;
	plt->m_flags = flags;
	if(beforeID > 0)
	{
	  pVectt->insertItemBefore(plt, beforeID);
	}
	else
	{
	  pVectt->insertItemAt(plt, beforeID);
	}
	return (XAP_Menu_Id) newID;
}

/*!
 * Remove the menu item with id nukeID from the menu labelled szMenu in the
 * Language set szLanguage
 */
XAP_Menu_Id XAP_Menu_Factory::removeMenuItem(const char * szMenu, 
							  const char * /*szLanguage*/,  
							  XAP_Menu_Id nukeID)
{
	UT_return_val_if_fail (szMenu && *szMenu, 0);		// no defaults
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		bFoundMenu = (g_ascii_strcasecmp(szMenu,pVectt->m_name)==0);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	pVectt->removeItem(nukeID);
	return nukeID;
}

/*!
 * Remove the menu item named szNuke from the menu labelled szMenu in the
 * Language set szLanguage
 */
XAP_Menu_Id XAP_Menu_Factory::removeMenuItem(const char * szMenu,
									  const char * /*szLanguage*/,
											 const char * szNuke)
{
	UT_return_val_if_fail (szMenu && *szMenu, 0);		// no defaults
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
		if (pVectt == NULL)
			continue;
		bFoundMenu = (g_ascii_strcasecmp(szMenu,pVectt->m_name)==0);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return 0;
	}
	
// next we need to find the id of the label name
//
// OK now search for this label
//
	UT_String Nuke = szNuke;
	XAP_Menu_Id nukeID = EV_searchMenuLabel( m_pLabelSet, Nuke);
	if(nukeID == 0)
	{
		if(m_pEnglishLabelSet == NULL)
		{
			buildBuiltInMenuLabelSet( m_pEnglishLabelSet);
		}
		nukeID = EV_searchMenuLabel( m_pEnglishLabelSet, Nuke);
		if(nukeID == 0)
		{	
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return 0;
		}
	}
//
// Got the id. Now remove it.
//
	pVectt->removeItem(nukeID);
	return nukeID;
}

void XAP_Menu_Factory::resetMenusToDefault(void)
{
    UT_VECTOR_PURGEALL(_vectt *,m_vecTT);
	m_vecTT.clear();
	UT_uint32 k = 0;
	for (k=0; k<G_N_ELEMENTS(s_ttTable); k++)
	{
		_vectt * pVectt = new _vectt(&s_ttTable[k]);
		m_vecTT.addItem(pVectt);
	}
}

/*!
 * Build a label set in memory that can be cloned by frames and added to by plugins.
 */
bool  XAP_Menu_Factory::buildMenuLabelSet(const char * szLanguage_)
{
	char buf[300];
	strncpy(buf,szLanguage_ ? szLanguage_ : "", sizeof(buf)-1);
	char* szLanguage = buf;

	char* dot = strrchr(szLanguage,'.');
	if (dot)
		*dot = '\0'; /* remove encoding part from locale name */

	UT_DEBUGMSG(("CreateMenuLabelSet: szLanguage_ %s, szLanguage %s\n"
				,szLanguage_,szLanguage));


	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	if( !m_pLabelSet )
	{
		m_pLabelSet = new EV_Menu_LabelSet(szLanguage,AP_MENU_ID__BOGUS1__,AP_MENU_ID__BOGUS2__);	
		std::string s1, s2;
		#define menuitem(id)                                                         \
		{                                                                            \
            pSS->getValueUTF8(AP_STRING_ID_MENU_LABEL_##id, s1);                     \
			pSS->getValueUTF8(AP_STRING_ID_MENU_STATUSLINE_##id, s2);                \
			m_pLabelSet->setLabel((AP_MENU_ID_##id), s1.c_str(), s2.c_str() ); \
	    }
		#include "ap_Menu_Id_List.h"
		#undef menuitem
		return true;
	}
	return false;
}


/*!
 * Build a builtin label set in memory that can be used by plugins to find where 
 to put menu items in non-Enmgish locales.
 * Do tricky reference to pointer here.... 
*/
bool  XAP_Menu_Factory::buildBuiltInMenuLabelSet(EV_Menu_LabelSet *& pLabelSet)
{
	if(m_pBSS == NULL)
	{
		AP_BuiltinStringSet * pBSS = new AP_BuiltinStringSet(m_pApp,"en-US");
		m_pBSS = (XAP_StringSet *) pBSS;
	}
	pLabelSet = new EV_Menu_LabelSet("en-US",AP_MENU_ID__BOGUS1__,AP_MENU_ID__BOGUS2__);	
	std::string s1, s2;
    #define menuitem(id)                                                       \
	{                                                                          \
	    m_pBSS->getValueUTF8(AP_STRING_ID_MENU_LABEL_##id, s1);                \
		m_pBSS->getValueUTF8(AP_STRING_ID_MENU_STATUSLINE_##id, s2);           \
		pLabelSet->setLabel( (AP_MENU_ID_##id),	s1.c_str(), s2.c_str() );\
	}
    #include "ap_Menu_Id_List.h"
    #undef menuitem
	return true;

}

EV_Menu_LabelSet *  XAP_Menu_Factory::CreateMenuLabelSet(const char * szLanguage_)
{
	char buf[300];
	strncpy(buf,szLanguage_ ? szLanguage_ : "", sizeof(buf)-1);
	char* szLanguage = buf;

	char* dot = strrchr(szLanguage,'.');
	if (dot)
		*dot = '\0'; /* remove encoding part from locale name */

	UT_DEBUGMSG(("CreateMenuLabelSet: szLanguage_ %s, szLanguage %s\n"
				,szLanguage_,szLanguage));


	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	if( !m_pLabelSet )
	{
		m_pLabelSet = new EV_Menu_LabelSet(szLanguage,AP_MENU_ID__BOGUS1__,AP_MENU_ID__BOGUS2__);

		std::string s1, s2;
		#define menuitem(id)                                                          \
		{                                                                             \
		    pSS->getValueUTF8(AP_STRING_ID_MENU_LABEL_##id, s1);                      \
			pSS->getValueUTF8(AP_STRING_ID_MENU_STATUSLINE_##id, s2);                 \
			m_pLabelSet->setLabel( (AP_MENU_ID_##id), s1.c_str(), s2.c_str() ); \
		}
			#include "ap_Menu_Id_List.h"
		#undef menuitem
	}

	return new EV_Menu_LabelSet(m_pLabelSet);
}

UT_uint32 XAP_Menu_Factory::GetMenuLabelSetLanguageCount(void)
{
	return 1;
}

const char * XAP_Menu_Factory::GetNthMenuLabelLanguageName(UT_uint32 /*ndx*/)
{
	return m_pLabelSet->getLanguage();
}

/*!
 * afterID has just been created. All labels of value equal or greater 
 * must be incremented
 */
bool XAP_Menu_Factory::addNewLabel(const char * /*szLanguage*/, 
										XAP_Menu_Id newID, 
										const char * szNewName,
										const char * szNewTooltip)
{
	// TODO incrementing label IDs??
	EV_Menu_Label * newLab = new EV_Menu_Label(newID,szNewName,szNewTooltip);
	return m_pLabelSet->addLabel(newLab);
}

bool XAP_Menu_Factory::removeLabel(const char * /*szLanguage*/, 
									   XAP_Menu_Id /*nukeID*/)
{
	return false;
}

void XAP_Menu_Factory::resetLabelsToDefault(void)
{
	// TODO - do nothing as extra labels in set don't hurt operation
}

EV_EditMouseContext XAP_Menu_Factory::createContextMenu(const char * szMenu)
{
	struct _lt items[2] =
	{{EV_MLF_BeginPopupMenu,AP_MENU_ID__BOGUS1__ },
	{EV_MLF_EndPopupMenu,AP_MENU_ID__BOGUS1__ }};
	struct _tt newtt;
	newtt.m_name = szMenu;
	newtt.m_nrEntries = 2;
	newtt.m_lt = items;
	newtt.m_emc = EV_EMC_AVAIL;
	/*
		We have several possibilities here:
		- replace the first NULL instance with scanning of all values,
		if any or add to the end;
		- add to the end in all cases;
		- any better algorithm ?
		I (jbrefort) choosed arbitrarily the first option.
	*/
	while (newtt.m_emc < m_NextContext)
	{
		if (m_vecTT.getNthItem (newtt.m_emc) == NULL)
			break;
		newtt.m_emc++;
	}
	newtt.m_emc = m_NextContext;
	_vectt * pVectt = new _vectt(&newtt);
	if (newtt.m_emc == m_NextContext)
	{
		m_vecTT.addItem(pVectt);
		m_NextContext++;
	}
	else
		m_vecTT.setNthItem (newtt.m_emc, pVectt, NULL);
	return newtt.m_emc;
}

void XAP_Menu_Factory::removeContextMenu(EV_EditMouseContext menuID)
{
	UT_sint32 k = 0;
	bool bFoundMenu = false;
	_vectt * pVectt = NULL;
	for (k=0; (k< m_vecTT.getItemCount()) && !bFoundMenu; k++)
	{
		pVectt = m_vecTT.getNthItem(k);
    if (pVectt == NULL)
			continue;
		bFoundMenu = (pVectt->m_emc==menuID);
	}
	if(!bFoundMenu)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	m_vecTT.deleteNthItem(k-1);
	DELETEP(pVectt);
}
