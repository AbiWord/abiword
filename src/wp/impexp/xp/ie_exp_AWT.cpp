/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ie_exp_AWT.h"
#include "ut_string.h"
#include "ut_assert.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_AWT_Sniffer::IE_Exp_AWT_Sniffer ()
	: IE_ExpSniffer(IE_IMPEXPNAME_AWML11AWT)
{
	// 
}

bool IE_Exp_AWT_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".awt"));
}

UT_Error IE_Exp_AWT_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_AWT * p = new IE_Exp_AWT(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_AWT_Sniffer::getDlgLabels(const char ** pszDesc,
											const char ** pszSuffixList,
											IEFileType * ft)
{
	*pszDesc = "AbiWord Template (.awt)";
	*pszSuffixList = "*.awt";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_AWT::IE_Exp_AWT (PD_Document * pDocument)
	: IE_Exp_AbiWord_1 (pDocument, true)
{
}

IE_Exp_AWT::~IE_Exp_AWT()
{
}

