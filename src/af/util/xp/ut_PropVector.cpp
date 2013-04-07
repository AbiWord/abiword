/* AbiSource Program Utilities
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2004 Hubert Figuiere
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


#include <string.h>

#include "ut_string.h"
#include "ut_PropVector.h"




void UT_PropVector::addOrReplaceProp(const gchar * pszProp, const gchar * pszVal)
{
	UT_ASSERT(pszVal);
	
	UT_sint32 iCount = getItemCount();
	const char * pszV = NULL;
/*	if(iCount <= 0)
	{
		char * prop = g_strdup(pszProp);
		char * val = g_strdup(pszVal);
		vec.addItem(static_cast<void *>(prop));
		vec.addItem(static_cast<void *>(val));
		return;
	}*/
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2) {
		pszV = reinterpret_cast<const gchar *>(getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0)) {
			break;
		}
	}
	if((iCount > 0) && (i < iCount)) {
	    gchar* pVal = NULL;
		gchar * val = g_strdup(pszVal);
		setNthItem(i+1, val, &pVal);
		FREEP(pVal);
	}
	else {
		gchar * prop = g_strdup(pszProp);
		gchar * val = g_strdup(pszVal);
		addItem(prop);
		addItem(val);
	}
	return;
}


void UT_PropVector::getProp(const gchar * pszProp,
									   const gchar * &pszVal) const
{
	UT_sint32 iCount = getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		pszVal = getNthItem(i+1);
	}
	return;
}

/*!
 Removes the key,value pair  (pszProp,pszVal) given by pszProp
 from the Vector of all properties of the current format.
 If the Property does not exists nothing happens
 \param UT_Vector &vec the vector to remove the pair from
 \param const gchar * pszProp the property name
*/
void UT_PropVector::removeProp(const gchar * pszProp)
{
	UT_sint32 iCount = getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0)) {
			break;
		}
	}
	if(i < iCount)
	{
		gchar * pSP = getNthItem(i);
		gchar * pSV = getNthItem(i+1);
		FREEP(pSP);
		FREEP(pSV);
		deleteNthItem(i+1);
		deleteNthItem(i);
	}
	return;
}

