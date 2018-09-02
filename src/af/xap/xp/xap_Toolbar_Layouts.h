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


#ifndef XAP_TOOLBAR_LAYOUTS_H
#define XAP_TOOLBAR_LAYOUTS_H

#include "ut_string_class.h"
#include "ut_vector.h"

#include "xap_Strings.h"

#include "ev_Toolbar_Layouts.h"
#include "ev_EditBits.h"

class XAP_App;

EV_Toolbar_Layout * AP_CreateToolbarLayout(const char * szName);

struct ABI_EXPORT XAP_Toolbar_Factory_lt
{
  EV_Toolbar_LayoutFlags	m_flags;
  XAP_Toolbar_Id			m_id;
};

struct ABI_EXPORT XAP_Toolbar_Factory_tt
{
  const char *				m_name;
  XAP_String_Id				m_label;
  const gchar*			m_prefKey;
  UT_uint32					m_nrEntries;
  XAP_Toolbar_Factory_lt *	m_lt;
};

class ABI_EXPORT XAP_Toolbar_Factory_vec
{
public:
  XAP_Toolbar_Factory_vec(const char * szName);
  XAP_Toolbar_Factory_vec(XAP_Toolbar_Factory_tt * orig);
  XAP_Toolbar_Factory_vec(EV_Toolbar_Layout *  orig);
  ~XAP_Toolbar_Factory_vec();
  UT_uint32 getNrEntries(void);
  void add_lt(XAP_Toolbar_Factory_lt * plt);
  XAP_Toolbar_Factory_lt * getNth_lt(UT_uint32 i);
  void insertLastItem(XAP_Toolbar_Factory_lt * p);
  void insertItemBefore(XAP_Toolbar_Factory_lt * p, XAP_Toolbar_Id id);
  void insertItemAfter(XAP_Toolbar_Factory_lt * p, XAP_Toolbar_Id id);
  bool removeToolbarId(XAP_Toolbar_Id id);
  const char * getToolbarName(void);
  XAP_String_Id getLabelStringID(void);
  const gchar * getPrefKey(void);
private:
  std::string m_name;
  XAP_String_Id m_label;
  const gchar*			m_prefKey;
  UT_GenericVector<XAP_Toolbar_Factory_lt *> m_Vec_lt;
};

class ABI_EXPORT XAP_Toolbar_Factory
{
public:

	XAP_Toolbar_Factory(XAP_App * pApp);
	~XAP_Toolbar_Factory(void);
	EV_Toolbar_Layout * CreateToolbarLayout(const char * szName);
	EV_Toolbar_Layout * DuplicateToolbarLayout(const char * szName);
	void             restoreToolbarLayout( EV_Toolbar_Layout * pTB);
	bool             addIconBefore(const char * szName,
								   XAP_Toolbar_Id newId,
								   XAP_Toolbar_Id beforeId);
	bool             addIconAfter(const char * szName,
								  XAP_Toolbar_Id newId,
								  XAP_Toolbar_Id afterId);
	bool             addIconAtEnd(const char * szName,
								  XAP_Toolbar_Id newId);
    bool             removeIcon(const char * szName,
									XAP_Toolbar_Id nukeId);
    bool             saveToolbarsInCurrentScheme(void);
    bool             restoreToolbarsFromCurrentScheme(void);
	const UT_GenericVector<UT_UTF8String*> & 	getToolbarNames(void);
	UT_uint32			countToolbars(void) const;
	const gchar*		prefKeyForToolbar(UT_uint32 t) const;
private:
  UT_GenericVector<XAP_Toolbar_Factory_vec*> m_vecTT;
  XAP_App * m_pApp;
  UT_GenericVector<UT_UTF8String*> m_tbNames;
};


#endif /* XAP_TOOLBAR_LAYOUTS_H */





