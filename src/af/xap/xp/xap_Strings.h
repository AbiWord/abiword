/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef XAP_STRINGS_H
#define XAP_STRINGS_H

#include <map>
#include <string>
#include <glib/gi18n.h>
#include <clocale>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_xml.h"
#include "ut_vector.h"
#include "ut_string.h"
#include "ut_string_class.h"

#define GETTEXT_PACKAGE "abiword"

typedef enum _XAP_String_Id_Enum
{
	XAP_STRING_ID__FIRST__			= 0,	/* must be first */
	XAP_STRING_ID__LAST__					/* must be last */
} XAP_String_Id_Enum;


#include "xap_String_Id.h"

//////////////////////////////////////////////////////////////////
// Both XAP_ and AP_ enum sets fold into XAP_String_Id
//////////////////////////////////////////////////////////////////

typedef const char*     XAP_String_Id;

//////////////////////////////////////////////////////////////////
// base class provides interface regardless of how we got the strings
//////////////////////////////////////////////////////////////////

class ABI_EXPORT XAP_StringSet
{
public:
	XAP_StringSet(const gchar * szDomainName);
	virtual ~XAP_StringSet(void);

	const gchar *			getLanguageName(void) const;

	virtual const gchar *	getValue(XAP_String_Id id) const = 0;

	bool getValue(XAP_String_Id id, const char * inEncoding, UT_String &s) const;
	bool getValueUTF8(XAP_String_Id id, std::string &s) const;
	bool getValueUTF8(XAP_String_Id id, UT_UTF8String &s) const;

	void setEncoding(const char * inEndcoding);
	const char * getEncoding() const;

	const char * setDomain(const char * szDomain);

protected:
	const gchar *			m_szLanguageName;
  const char * translate(XAP_String_Id id) const;

private:
  const gchar * m_domain;
	UT_String m_encoding ;
};

#endif /* XAP_STRINGS_H */
