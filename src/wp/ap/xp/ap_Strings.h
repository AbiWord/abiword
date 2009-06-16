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

#ifndef AP_STRINGS_H
#define AP_STRINGS_H

#include "xap_Strings.h"

//////////////////////////////////////////////////////////////////
// build a table of AP ID values
//////////////////////////////////////////////////////////////////

#define dcl(id,s)					AP_STRING_ID_##id,

/*typedef enum _AP_String_Id_Enum
{
	AP_STRING_ID__FIRST__			= 1000,	/* must be first -- must be >= XAP_STRING_ID__LAST__ */
#include "ap_String_Id.h" /*
	AP_STRING_ID__LAST__					/* must be last */
//} AP_String_Id_Enum;

#undef dcl

#endif /* AP_STRINGS_H */

class ABI_EXPORT AP_StringSet : public XAP_StringSet
{
  public:
    AP_StringSet(XAP_App *pApp, char *szLanguageName);
    virtual ~AP_StringSet(void);
    const char *getValue(XAP_String_Id id) const;
};
