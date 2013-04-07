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

#ifndef EV_MENU_H
#define EV_MENU_H

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

#include "ut_types.h"
#include "xap_Types.h"

class EV_EditMethodContainer;
class EV_EditMethod;
class EV_Menu_Layout;
class EV_Menu_LabelSet;
class EV_Menu_Action;
class EV_Menu_Label;
class AV_View;
class XAP_App;
class XAP_Frame;
class UT_String;

class ABI_EXPORT EV_Menu
{
public:
	EV_Menu(XAP_App* pApp,
			EV_EditMethodContainer * pEMC,
			const char * szMenuLayoutName,
			const char * szMenuLanguageName);
	virtual ~EV_Menu();

	bool invokeMenuMethod(AV_View * pView,
						  EV_EditMethod * pEM,
						  UT_UCSChar * pData,
						  UT_uint32 dataLength);

	bool invokeMenuMethod(AV_View* pView,
						  EV_EditMethod* pEM,
						  const UT_String& szScriptName);

	const EV_Menu_Layout *		getLayout() const { return m_pMenuLayout; }
	const EV_Menu_LabelSet *	getLabelSet() const { return m_pMenuLabelSet; }
	EV_Menu_Layout *			getLayout() { return m_pMenuLayout; }
	EV_Menu_LabelSet *			getLabelSet() { return m_pMenuLabelSet; }

	XAP_Menu_Id					addMenuItem(const UT_String& path, const UT_String& description);

protected:
	const char ** 				getLabelName(XAP_App * pApp,
											 const EV_Menu_Action * pAction, const EV_Menu_Label * pLabel);
	XAP_App *					getApp() { return m_pApp; }

// private: TODO: our inherited classes should have no business with our variables!
	EV_EditMethodContainer *	m_pEMC;
	EV_Menu_Layout *			m_pMenuLayout;	/* abstract ordering of our menu */
	EV_Menu_LabelSet *			m_pMenuLabelSet;/* strings (in a given language) for the menu */

protected:
	virtual bool				_doAddMenuItem(UT_uint32 layout_pos) = 0;

private:
	XAP_App *					m_pApp;
};

XAP_Menu_Id EV_searchMenuLabel(const EV_Menu_LabelSet* labels, const UT_String &label);

#endif /* EV_MENU_H */
