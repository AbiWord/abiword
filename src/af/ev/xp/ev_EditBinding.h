 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#ifndef EV_EDITBINDING_H
#define EV_EDITBINDING_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ev_EditBits.h"
#include "ev_EditMethod.h"

typedef UT_uint32 EV_EditBindingType;
#define EV_EBT_METHOD			((EV_EditBindingType) 0x1)
#define EV_EBT_PREFIX			((EV_EditBindingType) 0x2)

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
