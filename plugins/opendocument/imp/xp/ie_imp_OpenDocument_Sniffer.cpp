/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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
#include "ie_imp_OpenDocument_Sniffer.h"

// Internal includes
#include "ie_imp_OpenDocument.h"

#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>

/**
 * Constructor
 * 
 */
IE_Imp_OpenDocument_Sniffer::IE_Imp_OpenDocument_Sniffer () :
    IE_ImpSniffer("OpenDocument::ODT")
{
    
}

/**
 * Destructor
 * 
 */
IE_Imp_OpenDocument_Sniffer::~IE_Imp_OpenDocument_Sniffer ()
{
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_OpenDocument_Sniffer__SuffixConfidence[] = {
	{ "odt", 	UT_CONFIDENCE_PERFECT 	},
	{ "ott", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_OpenDocument_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_OpenDocument_Sniffer__SuffixConfidence;
}

// supported mimetypes
static IE_MimeConfidence IE_Imp_OpenDocument_Sniffer__MimeConfidence[] = {
	{ IE_MIME_MATCH_FULL, 	"application/vnd.oasis.opendocument.text", 	UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.oasis.opendocument.text-template", UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_FULL, 	"application/vnd.oasis.opendocument.text-web", UT_CONFIDENCE_GOOD 	},
	{ IE_MIME_MATCH_BOGUS, 	"", 										UT_CONFIDENCE_ZILCH }
};

const IE_MimeConfidence * IE_Imp_OpenDocument_Sniffer::getMimeConfidence ()
{
	return IE_Imp_OpenDocument_Sniffer__MimeConfidence;
}

/**
 * Recognize the contents as best we can
 * 
 */
UT_Confidence_t IE_Imp_OpenDocument_Sniffer::recognizeContents (GsfInput * input)
{
    UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;
	
	GsfInfile * zip  = gsf_infile_zip_new (input, NULL);
	if (zip != NULL) {
		GsfInput* pInput = gsf_infile_child_by_name(zip, "mimetype");
		
		if (pInput) {
			std::string mimetype;
			
			gsf_off_t size = gsf_input_size (pInput);
			if (size > 0) {
				const char * p = (const char *)gsf_input_read(pInput, size, NULL);
				if(p) {
					mimetype.assign(p, size);
				}
			}

			if ((mimetype == "application/vnd.oasis.opendocument.text") ||
				(mimetype == "application/vnd.oasis.opendocument.text-template") ||
				(mimetype == "application/vnd.oasis.opendocument.text-web")) {
				confidence = UT_CONFIDENCE_PERFECT;
			}
			
			g_object_unref (G_OBJECT (pInput));
		}
		else {
			// we need to figure out what to do without mimetype file.
			pInput = gsf_infile_child_by_name(zip, "content.xml");
			if(pInput) {
				// we need to identify further to get a better confidence.
				confidence = UT_CONFIDENCE_SOSO;
			}
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
UT_Error IE_Imp_OpenDocument_Sniffer::constructImporter (
                                        PD_Document* pDocument,
                                        IE_Imp** ppie)
{
    IE_Imp_OpenDocument* p = new IE_Imp_OpenDocument(pDocument);
    *ppie = p;
  
    return UT_OK;
}

/**
 * Get the dialog labels
 * 
 */
bool IE_Imp_OpenDocument_Sniffer::getDlgLabels (const char ** szDesc,
                          const char ** szSuffixList,
                          IEFileType * ft)
{
	*szDesc = "OpenDocument (.odt, .ott)";
	*szSuffixList = "*.odt; *.ott";
	*ft = getFileType();
  
    return true;
}
