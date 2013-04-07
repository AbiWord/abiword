/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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
#include <ie_imp_OpenXML_Sniffer.h>

// Internal includes
#include <ie_imp_OpenXML.h>

#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>

/**
 * Constructor
 * 
 */
IE_Imp_OpenXML_Sniffer::IE_Imp_OpenXML_Sniffer () :
    IE_ImpSniffer("OpenXML::OXML")
{
    
}

/**
 * Destructor
 * 
 */
IE_Imp_OpenXML_Sniffer::~IE_Imp_OpenXML_Sniffer ()
{
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_OpenXML_Sniffer__SuffixConfidence[] = {
	{ "docx", 	UT_CONFIDENCE_PERFECT 	},
	{ "dotx", 	UT_CONFIDENCE_PERFECT 	},
	{ "docm", 	UT_CONFIDENCE_PERFECT 	},
	{ "dotm", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_OpenXML_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_OpenXML_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_OpenXML_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	"application/vnd.openxmlformats-officedocument.wordprocessingml.document", 	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.openxmlformats-officedocument.wordprocessingml.template", 	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.ms-word.document", 	UT_CONFIDENCE_SOSO 	}, //will this interfere with doc import?
	{ IE_MIME_MATCH_FULL, 	"application/vnd.ms-word.template", 	UT_CONFIDENCE_SOSO 	},
	{ IE_MIME_MATCH_BOGUS, 	"", 										UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_OpenXML_Sniffer::getMimeConfidence ()
{
	return IE_Imp_OpenXML_Sniffer__MimeConfidence;
}

/**
 * Recognize the contents as best we can
 * 
 */
UT_Confidence_t IE_Imp_OpenXML_Sniffer::recognizeContents (GsfInput * input)
{
	UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;

	GsfInfile * zip;

	zip = gsf_infile_zip_new (input, NULL);
	if (zip != NULL)
	{
		GsfInput* pInput = gsf_infile_child_by_name(zip, "[Content_Types].xml");

		if (pInput) 
		{
			//For now we'll ignore parsing the content types file; just it being there is good enough.
			confidence = UT_CONFIDENCE_PERFECT;
			g_object_unref (G_OBJECT (pInput));
		}
		g_object_unref (G_OBJECT (zip));
	}

	return confidence;
}

/**
 * Construct an importer for ourselves
 * 
 */
UT_Error IE_Imp_OpenXML_Sniffer::constructImporter (
										PD_Document* pDocument,
										IE_Imp** ppie)
{
	IE_Imp_OpenXML* p = new IE_Imp_OpenXML(pDocument);
	*ppie = p;
  
	return UT_OK;
}

/**
 * Get the dialog labels
 * 
 */
bool IE_Imp_OpenXML_Sniffer::getDlgLabels (const char ** szDesc,
										const char ** szSuffixList,
										IEFileType * ft)
{
	*szDesc = "Office Open XML (.docx, .dotx, .docm, .dotm)";
	*szSuffixList = "*.docx; *.dotx; *.docm; *.dotm";
	*ft = getFileType();
  
	return true;
}

