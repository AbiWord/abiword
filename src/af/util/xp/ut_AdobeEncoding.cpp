/* AbiSource Program Utilities
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ut_AdobeEncoding.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


static int s_compare (const void * a, const void * b)
{
  const encoding_pair * ep;
  const char * name;

  name = (const char *) a;
  ep   = (const encoding_pair *) b;

  return UT_strcmp (name, ep->adb);
}


UT_AdobeEncoding::UT_AdobeEncoding(const encoding_pair * ep, UT_uint32 esize)
{
	m_pLUT = (encoding_pair*)ep;
	m_iLutSize = esize;
}


UT_UCSChar UT_AdobeEncoding::adobeToUcs(const char * str) const
{
	//first of all, see if the name is not of the uniXXXX type
	if(!strncmp(str,"uni",3) && isxdigit(*(str+3)) && isxdigit(*(str+4)) && isxdigit(*(str+5)) && isxdigit(*(str+6)))
	{
		char buff[7] = "0x";
		strcpy(buff + 2, str + 3);
		UT_uint32 i;
		sscanf(buff,"%x",&i);
		//printf("%x ", i);
		return ((UT_UCSChar) i);
	};
	
	encoding_pair * ep;
	ep = (encoding_pair *)bsearch (str, m_pLUT, m_iLutSize, sizeof (encoding_pair), s_compare);
   	if(ep)
   		return (ep->ucs);
   	else
   	{
   		UT_DEBUGMSG(("Unrecognised character: %s\n", str));
   		return 0;
   	}
}

const char * UT_AdobeEncoding::ucsToAdobe(const UT_UCSChar c)
{
	UT_uint32 i;
	
	for(i = 0; i < m_iLutSize; i++)
	{
		if(m_pLUT[i].ucs == c)
			return m_pLUT[i].adb;
	}
	
	/*	if we got this far, this char is not in our table, so we will
		produce a name in the uniXXXX format
	*/
	sprintf(m_buff, "uni%04x",(UT_uint32)c);
	return((const char *) m_buff);
}
