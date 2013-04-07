/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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

// Class definition include
#include <ie_exp_OpenXML_Sniffer.h>

// Internal includes
#include <ie_exp_OpenXML.h>

/**
 * Constructor
 * 
 */
IE_Exp_OpenXML_Sniffer::IE_Exp_OpenXML_Sniffer () :
    IE_ExpSniffer("OpenXML::OXML")
{
    
}

/**
 * Destructor
 * 
 */
IE_Exp_OpenXML_Sniffer::~IE_Exp_OpenXML_Sniffer ()
{
}

/**
 * Recognize the contents as best we can
 * 
 */
bool IE_Exp_OpenXML_Sniffer::recognizeSuffix (const char * szSuffix)
{
	if (g_ascii_strcasecmp(szSuffix,".docx") == 0)
	{
		return true;
	}
	if (g_ascii_strcasecmp(szSuffix,".dotx") == 0)
	{
		return true;
	}
	if (g_ascii_strcasecmp(szSuffix,".docm") == 0)
	{
		return true;
	}
	if (g_ascii_strcasecmp(szSuffix,".dotm") == 0)
	{
		return true;
	}
	return false;
}

UT_Confidence_t IE_Exp_OpenXML_Sniffer::supportsMIME(const char * szMIME)
{
	if (g_ascii_strcasecmp(szMIME, "application/vnd.openxmlformats-officedocument.wordprocessingml.document") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	if (g_ascii_strcasecmp(szMIME, "application/vnd.openxmlformats-officedocument.wordprocessingml.template") == 0)
	{
		return UT_CONFIDENCE_PERFECT;
	}
	if (g_ascii_strcasecmp(szMIME, "application/vnd.ms-word.document") == 0)
	{
		return UT_CONFIDENCE_SOSO;
	}
	if (g_ascii_strcasecmp(szMIME, "application/vnd.ms-word.template") == 0)
	{
		return UT_CONFIDENCE_SOSO;
	}
	return UT_CONFIDENCE_ZILCH;
}

/**
 * Construct an exporter for ourselves
 * 
 */
UT_Error IE_Exp_OpenXML_Sniffer::constructExporter (
										PD_Document* pDocument,
										IE_Exp** ppie)
{
	UT_DEBUGMSG(("FRT: Constructing an OpenXML Exporter\n"));
	IE_Exp_OpenXML* p = new IE_Exp_OpenXML(pDocument);
	*ppie = p;
  
	return UT_OK;
}

/**
 * Get the dialog labels
 * 
 */
bool IE_Exp_OpenXML_Sniffer::getDlgLabels (const char ** szDesc,
										const char ** szSuffixList,
										IEFileType * ft)
{
	*szDesc = "Office Open XML (.docx)";
	*szSuffixList = "*.docx";
	*ft = getFileType();
  
	return true;
}

