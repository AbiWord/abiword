/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002-2003 Dom Lachowicz 
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#include "ut_types.h"
#include "ut_string.h"
#include "ut_vector.h"
#include "ap_UnixClipboard.h"
#include "ap_UnixApp.h"
#include <vector>

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// RichText: cut and paste
#define AP_CLIPBOARD_TXT_RTF 			        "text/rtf"
#define AP_CLIPBOARD_APPLICATION_RTF            "application/rtf"

// HTML: cut only
#define AP_CLIPBOARD_TXT_HTML                   "text/html"
#define AP_CLIPBOARD_APPLICATION_XHTML          "application/xhtml+xml"

#define AP_CLIPBOARD_APPLICATION_ODT            "application/vnd.oasis.opendocument.text"

// Images: cut and paste
#define AP_CLIPBOARD_IMAGE_PNG                  "image/png"

// Images: paste only
#define AP_CLIPBOARD_IMAGE_JPEG                 "image/jpeg"
#define AP_CLIPBOARD_IMAGE_GIF                  "image/gif"
#define AP_CLIPBOARD_IMAGE_BMP                  "image/bmp"
#define AP_CLIPBOARD_IMAGE_TIFF                 "image/tiff"
#define AP_CLIPBOARD_IMAGE_WBMP                 "image/vnd.wap.wbmp"
#define AP_CLIPBOARD_IMAGE_XBM                  "image/x-xbitmap"
#define AP_CLIPBOARD_IMAGE_XPM                  "image/x-xpixmap"
#define AP_CLIPBOARD_IMAGE_PNM                  "image/x-portable-anymap"
#define AP_CLIPBOARD_IMAGE_PGM                  "image/x-portable-graymap"
#define AP_CLIPBOARD_IMAGE_PPM                  "image/x-portable-pixmap"
#define AP_CLIPBOARD_IMAGE_RAS                  "image/x-cmu-raster"
#define AP_CLIPBOARD_IMAGE_WMF                  "image/x-wmf"
#define AP_CLIPBOARD_IMAGE_SVG                  "image/svg"
#define AP_CLIPBOARD_IMAGE_SVG_XML              "image/svg+xml"
#define AP_CLIPBOARD_IMAGE_GOChart              "application/x-goffice-graph"

// Text: cut and paste
#define AP_CLIPBOARD_TEXT_UTF8_STRING           "UTF8_STRING"
#define AP_CLIPBOARD_TEXT                       "TEXT"
#define AP_CLIPBOARD_TEXT_STRING                "STRING"
#define AP_CLIPBOARD_TEXT_PLAIN                 "text/plain"
#define AP_CLIPBOARD_TEXT_COMPOUND              "COMPOUND_TEXT"

static const char * rtfszFormatsAccepted[] = {
  AP_CLIPBOARD_TXT_RTF,
  AP_CLIPBOARD_APPLICATION_RTF,
  AP_CLIPBOARD_APPLICATION_ODT,
  0 } ;

static const char * htmlszFormatsAccepted[] = {
  AP_CLIPBOARD_TXT_HTML,
  AP_CLIPBOARD_APPLICATION_XHTML,
  0 } ;

static const char * imgszFormatsAccepted[] = {
  AP_CLIPBOARD_IMAGE_GOChart,
  AP_CLIPBOARD_IMAGE_PNG,
  AP_CLIPBOARD_IMAGE_JPEG,
  AP_CLIPBOARD_IMAGE_TIFF,
  AP_CLIPBOARD_IMAGE_GIF,
  AP_CLIPBOARD_IMAGE_BMP,
  AP_CLIPBOARD_IMAGE_XBM,
  AP_CLIPBOARD_IMAGE_XPM,
  AP_CLIPBOARD_IMAGE_PNM,
  AP_CLIPBOARD_IMAGE_PPM,
  AP_CLIPBOARD_IMAGE_PGM,
  AP_CLIPBOARD_IMAGE_WBMP,
  AP_CLIPBOARD_IMAGE_RAS,
  AP_CLIPBOARD_IMAGE_WMF,
  AP_CLIPBOARD_IMAGE_SVG,
  AP_CLIPBOARD_IMAGE_SVG_XML,
  0 } ;

std::vector<const char*> vec_DynamicFormatsAccepted;

AP_UnixClipboard::AP_UnixClipboard(AP_UnixApp * pApp)
  : XAP_UnixClipboard(pApp)
{
  // DECLARE IN ORDER OF PREFERENCE RECEIVING

  // rich text types
  AddFmt(AP_CLIPBOARD_TXT_RTF);
  AddFmt(AP_CLIPBOARD_APPLICATION_RTF);
  
  // image types
  AddFmt ( AP_CLIPBOARD_IMAGE_GOChart ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_PNG ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_JPEG ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_TIFF ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_GIF ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_BMP ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_XBM ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_XPM ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_PNM ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_PPM ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_PGM ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_WBMP ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_RAS ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_WMF ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_SVG ) ;
  AddFmt ( AP_CLIPBOARD_IMAGE_SVG_XML ) ;

  // plain text types
  AddFmt(AP_CLIPBOARD_TEXT_UTF8_STRING);
  AddFmt(AP_CLIPBOARD_TEXT);
  AddFmt(AP_CLIPBOARD_TEXT_STRING);
  AddFmt(AP_CLIPBOARD_TEXT_PLAIN);
  AddFmt(AP_CLIPBOARD_TEXT_COMPOUND);

  // hypertext types
  AddFmt ( AP_CLIPBOARD_TXT_HTML ) ; // actually XHTML, but who's counting?
  AddFmt ( AP_CLIPBOARD_APPLICATION_XHTML ) ;
  vec_DynamicFormatsAccepted.insert(vec_DynamicFormatsAccepted.begin(), NULL);

  // O Dformat. This is provided by a plugin

  addFormat(AP_CLIPBOARD_APPLICATION_ODT);
}

bool AP_UnixClipboard::addTextData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes)
{
  if ( addData(tTo, AP_CLIPBOARD_TEXT_UTF8_STRING, pData, iNumBytes) &&
       addData(tTo, AP_CLIPBOARD_TEXT, pData, iNumBytes) &&
       addData(tTo, AP_CLIPBOARD_TEXT_STRING, pData, iNumBytes) &&
       addData(tTo, AP_CLIPBOARD_TEXT_PLAIN, pData, iNumBytes) &&
       addData(tTo, AP_CLIPBOARD_TEXT_COMPOUND,  pData, iNumBytes) )
    return true;
  return false;
}

bool AP_UnixClipboard::addRichTextData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes)
{
  if ( addData ( tTo, AP_CLIPBOARD_TXT_RTF, pData, iNumBytes ) && 
       addData ( tTo, AP_CLIPBOARD_APPLICATION_RTF, pData, iNumBytes ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::addHtmlData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes, bool xhtml)
{
  if (xhtml)
    {
      return addData (tTo, AP_CLIPBOARD_APPLICATION_XHTML, pData, iNumBytes) ? true : false;
    }
  return addData (tTo, AP_CLIPBOARD_TXT_HTML, pData, iNumBytes) ? true : false;
}

bool AP_UnixClipboard::addODTData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes)
{
  return addData ( tTo, AP_CLIPBOARD_APPLICATION_ODT, pData, iNumBytes );
}

bool AP_UnixClipboard::addPNGData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes)
{
  return addData ( tTo, AP_CLIPBOARD_IMAGE_PNG, pData, iNumBytes );
}

bool  AP_UnixClipboard::getSupportedData(T_AllowGet tFrom,
										 const void ** ppData, UT_uint32 * pLen,
										 const char **pszFormatFound)
{
	if (getData(tFrom, rtfszFormatsAccepted, (void**)ppData, pLen, pszFormatFound))
		return true;
	else if (getData (tFrom, htmlszFormatsAccepted, (void**)ppData, pLen, pszFormatFound))
		return true;
	else if (!vec_DynamicFormatsAccepted.empty() && getData(tFrom, &vec_DynamicFormatsAccepted[0], (void**)ppData, pLen, pszFormatFound))
		return true;  
	else if (getData(tFrom, imgszFormatsAccepted, (void**)ppData, pLen, pszFormatFound))
		return true;  
	else if (getTextData (tFrom, ppData, pLen, pszFormatFound))
		return true;
	return false;
}

bool  AP_UnixClipboard::getTextData(T_AllowGet tFrom,
									const void ** ppData, UT_uint32 * pLen,
									const char **pszFormatFound)
{
	bool rval = XAP_UnixClipboard::getTextData(tFrom, (void**)ppData, pLen);
	*pszFormatFound = "text/plain";
	return rval;
}

bool  AP_UnixClipboard::getRichTextData(T_AllowGet tFrom,
					const void ** ppData, UT_uint32 * pLen,
					const char **pszFormatFound)
{
  return getData( tFrom, rtfszFormatsAccepted, (void**)ppData, pLen, pszFormatFound ) ;
}

bool AP_UnixClipboard::getImageData(T_AllowGet tFrom,
									const void ** ppData, UT_uint32 * pLen,
									const char **pszFormatFound)
{
  return getData ( tFrom, imgszFormatsAccepted, (void**)ppData, pLen, pszFormatFound );
}

bool AP_UnixClipboard::getDynamicData(T_AllowGet tFrom,
			  const void ** ppData, UT_uint32 * pLen,
			  const char **pszFormatFound)
{
  return getData ( tFrom, &vec_DynamicFormatsAccepted[0], (void**)ppData, pLen, pszFormatFound );
}

bool AP_UnixClipboard::isTextTag ( const char * tag )
{
  if ( !tag || !strlen(tag) )
    return false ;

  if ( !g_ascii_strcasecmp( tag, AP_CLIPBOARD_TEXT_PLAIN ) ||
	   !g_ascii_strcasecmp( tag, AP_CLIPBOARD_TEXT_UTF8_STRING ) ||
       !g_ascii_strcasecmp( tag, AP_CLIPBOARD_TEXT ) ||
       !g_ascii_strcasecmp( tag, AP_CLIPBOARD_TEXT_STRING ) ||
       !g_ascii_strcasecmp( tag, AP_CLIPBOARD_TEXT_COMPOUND ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isRichTextTag ( const char * tag )
{
  if ( !tag || !strlen(tag) )
    return false ;

  if ( !g_ascii_strcasecmp ( tag, AP_CLIPBOARD_TXT_RTF ) ||
       !g_ascii_strcasecmp ( tag, AP_CLIPBOARD_APPLICATION_RTF ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isHTMLTag ( const char * tag )
{
  if ( !tag || !strlen(tag) )
    return false ;

  if ( !g_ascii_strcasecmp ( tag, AP_CLIPBOARD_TXT_HTML ) ||
       !g_ascii_strcasecmp ( tag, AP_CLIPBOARD_APPLICATION_XHTML ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isImageTag ( const char * tag )
{
  if ( !tag || !strlen(tag) )
    return false ;

  if ( !strncmp ( tag, "image/", 6 ) )
    return true ;

  if ( !strncmp ( tag, "application/x-goffice", 21 ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isDynamicTag ( const char * tag )
{
        if(vec_DynamicFormatsAccepted.empty())
	    return false;
	std::vector<const char*>::iterator i = vec_DynamicFormatsAccepted.begin();

	while (*i && strcmp (tag, *i))
		i++;
	return *i != NULL;
}

void AP_UnixClipboard::addFormat(const char * fmt)
{
	AddFmt(fmt);
	vec_DynamicFormatsAccepted.insert(vec_DynamicFormatsAccepted.begin(), fmt);
}

void AP_UnixClipboard::deleteFormat(const char * fmt)
{
	deleteFmt(fmt);
	std::vector<const char*>::iterator i = vec_DynamicFormatsAccepted.begin();
	while (*i && strcmp (fmt, *i))
		i++;
	if (*i)
		vec_DynamicFormatsAccepted.erase(i);
}
