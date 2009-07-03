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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_bytebuf.h"
#include "ut_wctomb.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "ap_Strings.h"

AP_StringSet::AP_StringSet(XAP_App *pApp, const char *szDomainName, const char *szLanguageName) :
  XAP_StringSet(pApp, szDomainName, szLanguageName)
{
}

AP_StringSet::~AP_StringSet(void)
{
}

const char *AP_StringSet::getValue(XAP_String_Id id) const
{
  return id;
}

const char *AP_StringSet::translate(XAP_String_Id id) const
{
  return _(id);
}
