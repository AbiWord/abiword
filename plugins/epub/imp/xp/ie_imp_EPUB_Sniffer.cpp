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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ie_imp_EPUB_Sniffer.h"

// Supported suffixes
static IE_SuffixConfidence IE_Imp_EPUB_Sniffer_SuffixConfidence[] =
{
{ "epub", UT_CONFIDENCE_PERFECT },
{ "", UT_CONFIDENCE_ZILCH } };

// Supported mimetypes
static IE_MimeConfidence IE_Imp_EPUB_Sniffer_MimeConfidence[] =
{
{ IE_MIME_MATCH_FULL, "application/epub+zip", UT_CONFIDENCE_GOOD },
{ IE_MIME_MATCH_BOGUS, "", UT_CONFIDENCE_ZILCH } };

IE_Imp_EPUB_Sniffer::IE_Imp_EPUB_Sniffer() :
    IE_ImpSniffer("EPUB::EPUB")
{
    UT_DEBUGMSG(("Constructing sniffer\n"));
}

IE_Imp_EPUB_Sniffer::~IE_Imp_EPUB_Sniffer()
{

}

const IE_SuffixConfidence * IE_Imp_EPUB_Sniffer::getSuffixConfidence()
{
    UT_DEBUGMSG(("Recognizing suffixes\n"));
    return IE_Imp_EPUB_Sniffer_SuffixConfidence;
}

const IE_MimeConfidence * IE_Imp_EPUB_Sniffer::getMimeConfidence()
{
    UT_DEBUGMSG(("Recognizing mime type\n"));
    return IE_Imp_EPUB_Sniffer_MimeConfidence;
}

UT_Confidence_t IE_Imp_EPUB_Sniffer::recognizeContents(GsfInput * input)
{
    UT_DEBUGMSG(("Recognizing contents\n"));
    GsfInfile* zip = gsf_infile_zip_new(input, NULL);
    UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;
    if (zip != NULL)
    {
        GsfInput* mimetype = gsf_infile_child_by_name(zip, "mimetype");

        if (mimetype != NULL)
        {
            UT_DEBUGMSG(("Opened 'mimetype' file\n"));
            size_t size = gsf_input_size(mimetype);

            if (size > 0)
            {
                UT_DEBUGMSG(("Reading 'mimetype' file contents\n"));
                gchar* pMime = (gchar*) gsf_input_read(mimetype, size, NULL);
                UT_UTF8String mimeStr;
                mimeStr.append(pMime, size);

                if (!strcmp(mimeStr.utf8_str(), EPUB_MIMETYPE))
                {
                    UT_DEBUGMSG(("RUDYJ: Found EPUB\n"));
                    confidence = UT_CONFIDENCE_PERFECT;
                }
            }

            g_object_unref(G_OBJECT(mimetype));
        }

        g_object_unref(G_OBJECT(zip));
    }

    return confidence;
}

UT_Error IE_Imp_EPUB_Sniffer::constructImporter(PD_Document * pDocument,
        IE_Imp ** ppie)
{
    UT_DEBUGMSG(("Constructing importer\n"));
    IE_Imp_EPUB* importer = new IE_Imp_EPUB(pDocument);
    *ppie = importer;

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
