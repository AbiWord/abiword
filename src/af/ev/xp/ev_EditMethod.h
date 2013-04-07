/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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

#ifndef EV_EDITMETHOD_H
#define EV_EDITMETHOD_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_string_class.h"

class AV_View;

/*****************************************************************
******************************************************************
** The EditMethod mechanism provides essentially a 'call-by-name'
** capability.  A key, mouse, menu, or toolbar event may be bound
** to a 'named' function (of type EV_EditMethod_Fn).
**
** EV_EditMethodType provides some crude type-checking (primarily
** this is used by the invoker to check if the function requires
** data ('insertData' can be bound to a key like 'a' but would have
** problems if bound to 'mouse-1-click').
**
** All EditMethod functions have a fixed calling sequence.  An
** instance of EV_EditMethodCallData passed to the function is
** used to provide all of the actual data that the function needs.
**
** A EV_EditMethod defines a "named function" and consists of the
** name, a pointer to the actual code, type-checking information,
** and a short description of the function.  [I debated whether it
** would be better to put all of these fields as static members of
** a class consisting of just the static function -- I'm not sure
** it matters one way or another.  Perhaps the current way will be
** a little easier when it comes time to localize the short
** descriptions....]
**
** A EV_EditMethodContainer is just that, a container for the
** EV_EditMethod's.  It provides simple lookup capability.  It
** currently has 2 tables of EditMethod's -- a static one (the set
** of our builtin functions) and a dynamic one (intended for use
** by JavaScript later).  The container provides the entire set
** of defined functions -- if a function isn't listed in the
** container, it cannot be found or bound to.
**
** The actual tables of defined functions are defined by the
** application and not in this library.
******************************************************************
*****************************************************************/

/*****************************************************************/
/*****************************************************************/

typedef UT_uint32 EV_EditMethodType;
#define EV_EMT_REQUIREDATA		(static_cast<EV_EditMethodType>(0x1))
#define EV_EMT_REQUIRE_SCRIPT_NAME		(static_cast<EV_EditMethodType>(0x2))
/* define a method that can be called without a frame. */
#define EV_EMT_APP_METHOD		(static_cast<EV_EditMethodType>(0x4))

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT EV_EditMethodCallData
{
public:
	EV_EditMethodCallData();
	EV_EditMethodCallData(const UT_UCSChar * pData, UT_uint32 dataLength);
	EV_EditMethodCallData(const char * pChar, UT_uint32 dataLength);
	EV_EditMethodCallData(const UT_String& stScriptName);
	~EV_EditMethodCallData();

	UT_sint32			getX() { return m_xPos; }
	UT_sint32			getY() { return m_yPos; }
	const UT_String&	getScriptName() { return m_stScriptName; }

// private:
	UT_UCSChar *		m_pData;
	UT_uint32			m_dataLength;
	bool				m_bAllocatedData;
	UT_sint32			m_xPos;
	UT_sint32			m_yPos;

private:
	UT_String			m_stScriptName;
};

typedef bool (*EV_EditMethod_pCtxtFn)(AV_View * pView, EV_EditMethodCallData * pCallData,
									  void * context);
typedef bool ( EV_EditMethod_CtxtFn) (AV_View * pView, EV_EditMethodCallData * pCallData,
									  void * context);

typedef bool (*EV_EditMethod_pFn)(AV_View * pView, EV_EditMethodCallData * pCallData);
typedef bool ( EV_EditMethod_Fn) (AV_View * pView, EV_EditMethodCallData * pCallData);

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT EV_EditMethod
{
public:
	EV_EditMethod(const char * szName, EV_EditMethod_pFn fn, EV_EditMethodType emt,
				  const char * szDescription);
	EV_EditMethod(const char * szName, EV_EditMethod_pCtxtFn fn, EV_EditMethodType emt,
				  const char * szDescription, void * context);

	bool 					Fn(AV_View * pView, EV_EditMethodCallData * pCallData) const;

	EV_EditMethodType		getType() const;
	const char *			getName() const;
	const char *			getDescription() const;

protected:
	const char *			m_szName; // used for lookup; not malloced; should not be localized
	EV_EditMethod_pFn		m_fn;
	EV_EditMethod_pCtxtFn	m_CtxtFn;
	EV_EditMethodType		m_emt;
	const char *			m_szDescription; // not malloced; this can be localized
	void *					m_context;
};

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT EV_EditMethodContainer
{
public:
	EV_EditMethodContainer(UT_uint32 cStatic, EV_EditMethod arrayStaticEditMethods[]);
	~EV_EditMethodContainer();

	bool				addEditMethod(EV_EditMethod * pem);
	bool removeEditMethod(EV_EditMethod * pem);
	UT_uint32			countEditMethods();
	EV_EditMethod *		getNthEditMethod(UT_uint32 ndx);
	EV_EditMethod *		findEditMethodByName(const char * szName) const;

protected:
	UT_uint32			m_countStatic;
	EV_EditMethod *		m_arrayStaticEditMethods;		// not malloced
	UT_GenericVector<EV_EditMethod *>	m_vecDynamicEditMethods;
};

/*****************************************************************/
/*****************************************************************/

ABI_EXPORT bool ev_EditMethod_exists (const char * methodName);
ABI_EXPORT bool ev_EditMethod_exists (const UT_String & methodName);

/*****************************************************************/
/*****************************************************************/

ABI_EXPORT bool ev_EditMethod_invoke (const EV_EditMethod * pEM, EV_EditMethodCallData * pData);
ABI_EXPORT bool ev_EditMethod_invoke (const EV_EditMethod * pEM, const UT_String & data);
ABI_EXPORT bool ev_EditMethod_invoke (const EV_EditMethod * pEM, const UT_UCS4String & data);

ABI_EXPORT bool ev_EditMethod_invoke (const char * methodName, const UT_String & data);
ABI_EXPORT bool ev_EditMethod_invoke (const char * methodName, const UT_UCS4String & data);
ABI_EXPORT bool ev_EditMethod_invoke (const char * methodName, const char * data);
ABI_EXPORT bool ev_EditMethod_invoke (const char * methodName, const UT_UCSChar * data);
ABI_EXPORT bool ev_EditMethod_invoke (const UT_String& methodName, const UT_String & data);
ABI_EXPORT bool ev_EditMethod_invoke (const UT_String& methodName, const UT_UCS4String & data);

ABI_EXPORT EV_EditMethod* ev_EditMethod_lookup (const char * methodName);
ABI_EXPORT EV_EditMethod* ev_EditMethod_lookup (const UT_String & methodName);

#endif /* EV_EDITMETHOD_H */
