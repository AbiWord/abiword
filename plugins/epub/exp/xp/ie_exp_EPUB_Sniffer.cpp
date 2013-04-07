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

#include "ie_exp_EPUB_Sniffer.h"

IE_Exp_EPUB_Sniffer::IE_Exp_EPUB_Sniffer() :
    IE_ExpSniffer("EPUB201::EPUB")
{

}

IE_Exp_EPUB_Sniffer::~IE_Exp_EPUB_Sniffer()
{

}

bool IE_Exp_EPUB_Sniffer::recognizeSuffix(const char * szSuffix)
{
    return (!g_ascii_strcasecmp(szSuffix, ".epub"));
}

UT_Error IE_Exp_EPUB_Sniffer::constructExporter(PD_Document * pDocument,
        IE_Exp ** ppie)
{
    IE_Exp_EPUB * exporter = new IE_Exp_EPUB(pDocument);
    *ppie = exporter;
    return UT_OK;
}

bool IE_Exp_EPUB_Sniffer::getDlgLabels(const char ** pszDesc,
        const char ** pszSuffixList, IEFileType * ft)
{
    *pszDesc = "EPUB (.epub)";
    *pszSuffixList = "*.epub";
    *ft = getFileType();
    return true;
}
