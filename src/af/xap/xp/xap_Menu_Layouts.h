/* AbiSource Application Framework
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


#ifndef XAP_MENU_LAYOUTS_H
#define XAP_MENU_LAYOUTS_H

#include "ev_Menu_Layouts.h"
/* #include "ev_Menu_Labels.h" */
#include "ev_EditBits.h"
#include "xap_Features.h"

class EV_Menu_LabelSet;

class XAP_App;
class XAP_StringSet;

class _vectt;

class ABI_EXPORT XAP_Menu_Factory
{
public:

	XAP_Menu_Factory(XAP_App * pApp);
	~XAP_Menu_Factory(void);
	EV_Menu_Layout * CreateMenuLayout(const char * szName);
	const char * FindContextMenu(EV_EditMouseContext emc);
	XAP_Menu_Id			addNewMenuAfter(const char * szMenu,
									   const char * szLanguage,
									   const XAP_Menu_Id afterID,
									   EV_Menu_LayoutFlags flags,
									   XAP_Menu_Id newID = 0);
	XAP_Menu_Id        addNewMenuAfter(const char * szMenu,
									   const char * szLanguage,
									   const char * szAfter,
									   EV_Menu_LayoutFlags flags,
									   XAP_Menu_Id menuID = 0);
	XAP_Menu_Id			addNewMenuBefore(const char * szMenu,
									   const char * szLanguage,
									   const XAP_Menu_Id beforeID,
									   EV_Menu_LayoutFlags flags,
									   XAP_Menu_Id newID = 0);
	XAP_Menu_Id         addNewMenuBefore(const char * szMenu,
										 const char * szLanguage,
										 const char * szBefore,
									   EV_Menu_LayoutFlags flags, XAP_Menu_Id menuID = 0);
	XAP_Menu_Id        getNewID(void);
    XAP_Menu_Id        removeMenuItem(const char * szMenu,
									  const char * szLanguage,
									  XAP_Menu_Id nukeID);
    XAP_Menu_Id        removeMenuItem(const char * szMenu,
									  const char * szLanguage,
									  const char * szNuke);
	void         resetMenusToDefault(void);
	UT_uint32    GetMenuLabelSetLanguageCount(void);
	const char * GetNthMenuLabelLanguageName(UT_uint32 ndx);
	EV_Menu_LabelSet *  CreateMenuLabelSet(const char * szLanguage_);
	bool         buildMenuLabelSet(const char * szLanguage_);
	bool         buildBuiltInMenuLabelSet(  EV_Menu_LabelSet *& pLabelSet);
	bool         addNewLabel(const char * szMenu,
								  XAP_Menu_Id newID,
								  const char * szNewLabel,
								  const char * szNewTooltip);
	bool         removeLabel(const char * szMenu,
							 XAP_Menu_Id nukeID);

	void         resetLabelsToDefault(void);

	EV_EditMouseContext createContextMenu(const char * szMenu);
	void removeContextMenu(EV_EditMouseContext menuID);

private:

  UT_GenericVector<_vectt *> m_vecTT;
  XAP_App * m_pApp;
  EV_Menu_LabelSet * m_pLabelSet;
  EV_Menu_LabelSet * m_pEnglishLabelSet;
  XAP_Menu_Id m_maxID;
  XAP_StringSet * m_pBSS;
  EV_EditMouseContext m_NextContext;
};
#endif /* XAP_MENU_LAYOUTS_H */
