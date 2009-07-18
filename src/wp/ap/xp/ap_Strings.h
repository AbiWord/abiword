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

#define dcl(id,s) static const char * AP_STRING_ID_##id = s;
#include "ap_String_Id.h"
#undef dcl

class ABI_EXPORT AP_StringSet : public XAP_StringSet
{
  public:
    AP_StringSet(const char *szDomainName = NULL);
    virtual ~AP_StringSet(void);
    virtual const char *getValue(XAP_String_Id id) const;
};

#endif /* AP_STRINGS_H */
