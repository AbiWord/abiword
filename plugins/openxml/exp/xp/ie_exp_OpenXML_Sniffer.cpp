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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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

// supported suffixes
static IE_SuffixConfidence IE_Exp_OpenXML_Sniffer__SuffixConfidence[] = {
	{ "docx", 	UT_CONFIDENCE_PERFECT 	},
	{ "dotx", 	UT_CONFIDENCE_PERFECT 	},
	{ "docm", 	UT_CONFIDENCE_PERFECT 	},
	{ "dotm", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Exp_OpenXML_Sniffer::getSuffixConfidence ()
{
	return IE_Exp_OpenXML_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Exp_OpenXML_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	"application/vnd.openxmlformats-officedocument.wordprocessingml.document", 	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.openxmlformats-officedocument.wordprocessingml.template", 	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.ms-word.document", 	UT_CONFIDENCE_SOSO 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.ms-word.template", 	UT_CONFIDENCE_SOSO 	},
	{ IE_MIME_MATCH_BOGUS, 	"", 										UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Exp_OpenXML_Sniffer::getMimeConfidence ()
{
	return IE_Exp_OpenXML_Sniffer__MimeConfidence;
}

/**
 * Recognize the contents as best we can
 * 
 */
bool IE_Exp_OpenXML_Sniffer::recognizeSuffix (const char * /*szSuffix*/)
{
	return true;
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
	*szDesc = "OpenXML (.docx, .dotx, .docm, .dotm)";
	*szSuffixList = "*.docx; *.dotx; *.docm; *.dotm";
	*ft = getFileType();
  
	return true;
}

