/* AbiWord
 * Copyright (C) 1998-2002 AbiSource, Inc.
 * Copyright (C) 2002 Dom Lachowicz 
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#include "ut_types.h"
#include "ut_vector.h"
#include "ap_UnixClipboard.h"
#include "ap_UnixApp.h"

#include <gdk/gdk.h>

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// cut and paste
#define AP_CLIPBOARD_TXT_RTF 			"text/rtf"
#define AP_CLIPBOARD_APPLICATION_RTF            "application/rtf"

// cut only
#define AP_CLIPBOARD_TXT_HTML                   "text/html"
#define AP_CLIPBOARD_APPLICATION_XHTML          "application/xhtml+xml"

// paste only
#define AP_CLIPBOARD_IMAGE_PNG                  "image/png"
#define AP_CLIPBOARD_IMAGE_JPEG                 "image/jpeg"
#define AP_CLIPBOARD_IMAGE_GIF                  "image/gif"
#define AP_CLIPBOARD_IMAGE_BMP                  "image/bmp"
#define AP_CLIPBOARD_IMAGE_TIFF                 "image/tiff"

// text cut+paste handled by GTK+2 at the XAP layer

static const char * rtfszFormatsAccepted[] = {
  AP_CLIPBOARD_TXT_RTF,
  AP_CLIPBOARD_APPLICATION_RTF,
  0 } ;

static const char * imgszFormatsAccepted[] = {
  AP_CLIPBOARD_IMAGE_PNG,
  AP_CLIPBOARD_IMAGE_JPEG,
  AP_CLIPBOARD_IMAGE_GIF,
  AP_CLIPBOARD_IMAGE_BMP,
  AP_CLIPBOARD_IMAGE_TIFF,
  0 } ;

AP_UnixClipboard::AP_UnixClipboard(AP_UnixApp * pApp)
  : XAP_UnixClipboard(pApp)
{
        // rich text types
   	AddFmt(AP_CLIPBOARD_TXT_RTF);
	AddFmt(AP_CLIPBOARD_APPLICATION_RTF);

	// hypertext types
	AddFmt ( AP_CLIPBOARD_TXT_HTML ) ; // actually XHTML, but who's counting?
	AddFmt ( AP_CLIPBOARD_APPLICATION_XHTML ) ;

	// image types
	AddFmt ( AP_CLIPBOARD_IMAGE_PNG ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_JPEG ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_GIF ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_BMP ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_TIFF ) ;
}

bool AP_UnixClipboard::addTextData(void* pData, UT_sint32 iNumBytes)
{
  return addTextUTF8(pData, iNumBytes);
}

bool AP_UnixClipboard::addRichTextData(void* pData, UT_sint32 iNumBytes)
{
  if ( addData ( AP_CLIPBOARD_TXT_RTF, pData, iNumBytes ) && 
       addData ( AP_CLIPBOARD_APPLICATION_RTF, pData, iNumBytes ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::addHtmlData(void* pData, UT_sint32 iNumBytes)
{
  if ( addData ( AP_CLIPBOARD_TXT_HTML, pData, iNumBytes ) &&
        addData ( AP_CLIPBOARD_APPLICATION_XHTML, pData, iNumBytes ) )
    return true ;
  return false ;

}

bool  AP_UnixClipboard::getSupportedData(T_AllowGet tFrom,
					 void ** ppData, UT_uint32 * pLen,
					 const char **pszFormatFound)
{
  // give priority to rich text, then text, then images
  if (getData(tFrom, rtfszFormatsAccepted, ppData, pLen, pszFormatFound))
    return true;
  else if (getTextData (tFrom, ppData, pLen, pszFormatFound))
    return true;
  else if (getData(tFrom, imgszFormatsAccepted, ppData, pLen, pszFormatFound))
    return true;

  return false;
}

bool  AP_UnixClipboard::getTextData(T_AllowGet tFrom,
				    void ** ppData, UT_uint32 * pLen,
				    const char **pszFormatFound)
{
  bool bRet = getTextUTF8(tFrom, ppData, pLen);

  // GTK+ returns everything in utf8, so let's just fake text-plain and let
  // things handle themselves :)
  if (bRet)
    *pszFormatFound = "text/plain";
  else
    *pszFormatFound = "";
  return bRet;
}

bool  AP_UnixClipboard::getRichTextData(T_AllowGet tFrom,
					void ** ppData, UT_uint32 * pLen,
					const char **pszFormatFound)
{
  return getData( tFrom, rtfszFormatsAccepted, ppData, pLen, pszFormatFound ) ;
}

bool AP_UnixClipboard::getImageData(T_AllowGet tFrom,
				    void ** ppData, UT_uint32 * pLen,
				    const char **pszFormatFound)
{
  return getData ( tFrom, imgszFormatsAccepted, ppData, pLen, pszFormatFound );
}

bool AP_UnixClipboard::isTextTag ( const char * tag )
{
  if ( !tag && !strlen(tag) )
    return false ;

  // getTextData will only return this because it's sort-of a hack
  if ( !strcmp( tag, "text/plain" ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isRichTextTag ( const char * tag )
{
  if ( !tag && !strlen(tag) )
    return false ;

  if ( !strcmp ( tag, AP_CLIPBOARD_TXT_RTF ) ||
       !strcmp ( tag, AP_CLIPBOARD_APPLICATION_RTF ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isHTMLTag ( const char * tag )
{
  if ( !tag && !strlen(tag) )
    return false ;

  if ( !strcmp ( tag, AP_CLIPBOARD_TXT_HTML ) ||
       !strcmp ( tag, AP_CLIPBOARD_APPLICATION_XHTML ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isImageTag ( const char * tag )
{
  if ( !tag && !strlen(tag) )
    return false ;

  if ( !strncmp ( tag, "image/", 6 ) )
    return true ;
  return false ;
}
