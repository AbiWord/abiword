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

#ifndef EV_TOOLBAR_H
#define EV_TOOLBAR_H

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

#include "ut_types.h"

class EV_EditMethodContainer;
class EV_EditMethod;
class EV_Toolbar_Layout;
class EV_Toolbar_LabelSet;
class AV_View;

class ABI_EXPORT EV_Toolbar
{
public:
	EV_Toolbar(EV_EditMethodContainer * pEMC,
			   const char * szMenuLayoutName,
			   const char * szMenuLanguageName);
	virtual ~EV_Toolbar(void);

	const EV_Toolbar_Layout * getToolbarLayout(void) const;
	const EV_Toolbar_LabelSet * getToolbarLabelSet(void) const;

	bool invokeToolbarMethod(AV_View * pView,
							 EV_EditMethod * pEM,
							 const UT_UCS4Char * pData,
							 UT_uint32 dataLength);

	virtual bool synthesize(void) { return (false); } // Abstract
	virtual void show(void) { m_bHidden = false; } // It must be abstract, but I don't want to screw
	virtual void hide(void) { m_bHidden = true; } // the platforms that don't implement show/hide
	virtual bool repopulateStyles(void)  {return false;}
	// tells if the toolbar is hidden
	bool isHidden(void) const { return m_bHidden; };
protected:
	EV_EditMethodContainer *	m_pEMC;
	EV_Toolbar_Layout *			m_pToolbarLayout;
	EV_Toolbar_LabelSet *		m_pToolbarLabelSet;
private:
	bool							m_bHidden;
};

#endif /* EV_TOOLBAR_H */
