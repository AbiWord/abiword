/* AbiSource
 * 
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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



#include "ie_imp_EPUB_Sniffer.h"

// Supported suffixes
static IE_SuffixConfidence IE_Imp_EPUB_Sniffer_SuffixConfidence[] = 
{
	{ "epub", 	UT_CONFIDENCE_PERFECT 	},
	{ "",           UT_CONFIDENCE_ZILCH 	}
};

// Supported mimetypes
static IE_MimeConfidence IE_Imp_EPUB_Sniffer_MimeConfidence[] = 
{
	{ IE_MIME_MATCH_FULL, 	"application/epub+zip", UT_CONFIDENCE_GOOD  },
	{ IE_MIME_MATCH_BOGUS, 	"",                     UT_CONFIDENCE_ZILCH }
};

IE_Imp_EPUB_Sniffer::IE_Imp_EPUB_Sniffer() :
	IE_ImpSniffer("EPUB::EPUB") 
{

}

IE_Imp_EPUB_Sniffer::~IE_Imp_EPUB_Sniffer() 
{

}

const IE_SuffixConfidence * IE_Imp_EPUB_Sniffer::getSuffixConfidence() 
{
    return IE_Imp_EPUB_Sniffer_SuffixConfidence;
}

const IE_MimeConfidence * IE_Imp_EPUB_Sniffer::getMimeConfidence() 
{
	return IE_Imp_EPUB_Sniffer_MimeConfidence;
}

UT_Confidence_t IE_Imp_EPUB_Sniffer::recognizeContents(GsfInput * input) 
{
    GsfInfile* zip = gsf_infile_zip_new(input, NULL);
    UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;
    if (zip != NULL)
    {
        GsfInput* mimetype = gsf_infile_child_by_name(zip, "mimetype");
        
        if (mimetype != NULL)
        {
            gsf_off_t size = gsf_input_size(mimetype);
            
            if (size > 0)
            {
                gchar* mime = (gchar*)gsf_input_read(mimetype, size, NULL);

                if (!strcmp(mime, EPUB_MIMETYPE))
                {
                    confidence = UT_CONFIDENCE_PERFECT;
                }
                g_free(mime);
            }
        }        
    }
    
    return confidence;
}

UT_Error IE_Imp_EPUB_Sniffer::constructImporter(PD_Document * pDocument,
		IE_Imp ** ppie) 
{
    *ppie = new IE_Imp_EPUB(pDocument);
    
    return UT_OK;
}

bool IE_Imp_EPUB_Sniffer::getDlgLabels(const char ** szDesc,
		const char ** szSuffixList, IEFileType * ft) 
{
	*szDesc = "EPUB (.epub)";
	*szSuffixList = "*.epub";
	*ft = getFileType();
	return true;
}
