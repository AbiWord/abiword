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
 



#ifndef EV_EDITBINDING_H
#define EV_EDITBINDING_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ev_EditBits.h"
#include "ev_EditMethod.h"

typedef UT_uint32 EV_EditBindingType;
#define EV_EBT_METHOD			((EV_EditBindingType) 0x1) /* final method */
#define EV_EBT_PREFIX			((EV_EditBindingType) 0x2) /* prefix state (like ^X in emacs */

class EV_EditBindingMap;

/*****************************************************************/
/*****************************************************************/

class EV_EditBinding
{
public:
	EV_EditBinding(EV_EditBindingMap * pebm);
	EV_EditBinding(EV_EditMethod * pem);

	EV_EditBindingType				getType(void) const;
	EV_EditBindingMap *				getMap(void) const;
	EV_EditMethod *					getMethod(void) const;

protected:
	EV_EditBindingType				m_ebt;		/* discriminant for union */
	union _u
	{
		EV_EditBindingMap *			m_pebm;		/* we should destroy *//* this binding is a prefix */
		EV_EditMethod *				m_pem;		/* we should not destroy *//* this binding completes sequence */
	} u;
};

/*****************************************************************/
/*****************************************************************/

class ev_EB_MouseTable;
class ev_EB_NVK_Table;
class ev_EB_Char_Table;

/*****************************************************************/
/*****************************************************************/

class EV_EditBindingMap
{
public:
	EV_EditBindingMap(EV_EditMethodContainer * pemc);
	~EV_EditBindingMap();

	EV_EditBinding *	findEditBinding(EV_EditBits eb);
	UT_Bool				setBinding(EV_EditBits eb, const char * szMethodName);
	UT_Bool				setBinding(EV_EditBits eb, EV_EditBinding * peb);
	UT_Bool				removeBinding(EV_EditBits eb);
	UT_Bool				parseEditBinding(void);

	const char *		getShortcutFor(const EV_EditMethod * pEM) const;

protected:
	EV_EditMethodContainer *	m_pemc;

	ev_EB_MouseTable *			m_pebMT[EV_COUNT_EMB];
	ev_EB_NVK_Table *			m_pebNVK;
	ev_EB_Char_Table *			m_pebChar;
};

/*****************************************************************/
/*****************************************************************/

UT_Bool EV_LoadBindings_Default(EV_EditMethodContainer * pemc,
								EV_EditBindingMap **ppebm);

#endif /* EV_EDITBINDING_H */
