/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <gdk/gdk.h>

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define	AP_CLIPBOARD_TEXTPLAIN_8BIT 		"TEXT"
#define AP_CLIPBOARD_STRING			"STRING"
#define AP_CLIPBOARD_COMPOUND_TEXT		"COMPOUND_TEXT"
#define AP_CLIPBOARD_TEXT_PLAIN                 "text/plain"
#define AP_CLIPBOARD_TXT_RTF 			"text/rtf"
#define AP_CLIPBOARD_APPLICATION_RTF            "application/rtf"
#define AP_CLIPBOARD_TXT_HTML                   "text/html"
#define AP_CLIPBOARD_APPLICATION_XHTML          "application/xhtml+xml"

#define AP_CLIPBOARD_IMAGE_PNG "image/png"
#define AP_CLIPBOARD_IMAGE_JPEG "image/jpeg"
#define AP_CLIPBOARD_IMAGE_GIF "image/gif"
#define AP_CLIPBOARD_IMAGE_BMP "image/bmp"
#define AP_CLIPBOARD_IMAGE_TIFF "image/tiff"

static const char * txtszFormatsAccepted[] = { 
  AP_CLIPBOARD_STRING,
  AP_CLIPBOARD_TEXTPLAIN_8BIT,
  AP_CLIPBOARD_TEXT_PLAIN,
  AP_CLIPBOARD_COMPOUND_TEXT,
  0 };

static const char * imgszFormatsAccepted[] = {
  AP_CLIPBOARD_IMAGE_PNG,
  AP_CLIPBOARD_IMAGE_JPEG,
  AP_CLIPBOARD_IMAGE_GIF,
  AP_CLIPBOARD_IMAGE_BMP,
  AP_CLIPBOARD_IMAGE_TIFF,
  0 } ;

/*
  I've reordered AP_CLIPBOARD_STRING and AP_CLIPBOARD_TEXTPLAIN_8BIT
  since for non-Latin1 text the data in AP_CLIPBOARD_TEXTPLAIN_8BIT
  format has name of encoding as prefix, and AP_CLIPBOARD_STRING
  doesn't - hvv.
*/
static const char * aszFormatsAccepted[] = { 
  AP_CLIPBOARD_TXT_RTF,
  AP_CLIPBOARD_APPLICATION_RTF,
  AP_CLIPBOARD_STRING,
  AP_CLIPBOARD_TEXTPLAIN_8BIT,
  AP_CLIPBOARD_TEXT_PLAIN,
  AP_CLIPBOARD_COMPOUND_TEXT,

  AP_CLIPBOARD_IMAGE_PNG,
  AP_CLIPBOARD_IMAGE_JPEG,
  AP_CLIPBOARD_IMAGE_GIF,
  AP_CLIPBOARD_IMAGE_BMP,
  AP_CLIPBOARD_IMAGE_TIFF,

  0 /* must be last */ };

AP_UnixClipboard::AP_UnixClipboard(AP_UnixApp * pApp)
	: XAP_UnixClipboard((XAP_UnixApp *)(pApp))
{
#define AddFmt(szFormat)															\
	do {	m_vecFormat_AP_Name.addItem((void *) szFormat);							\
			m_vecFormat_GdkAtom.addItem((void *) gdk_atom_intern(szFormat,FALSE));	\
	} while (0)

        // rich text types
   	AddFmt(AP_CLIPBOARD_TXT_RTF);
	AddFmt(AP_CLIPBOARD_APPLICATION_RTF);

	// plain text types
	AddFmt(AP_CLIPBOARD_TEXTPLAIN_8BIT);
	AddFmt(AP_CLIPBOARD_STRING);		// alias for TEXTPLAIN_8BIT
	AddFmt(AP_CLIPBOARD_TEXT_PLAIN);
	AddFmt(AP_CLIPBOARD_COMPOUND_TEXT);

	// image types
	AddFmt ( AP_CLIPBOARD_IMAGE_PNG ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_JPEG ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_GIF ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_BMP ) ;
	AddFmt ( AP_CLIPBOARD_IMAGE_TIFF ) ;

	// TODO: deal with html

	// TODO deal with multi-byte text (either unicode or utf8 or whatever)
	// TODO add something like the following.  you should be able to test
	// TODO against xemacs.
	// TODO
	// TODO AddFmt(AP_CLIPBOARD_COMPOUND_TEXT);

#undef AddFmt
}

bool AP_UnixClipboard::addTextData(void* pData, UT_sint32 iNumBytes)
{
  if ( addData ( AP_CLIPBOARD_TEXTPLAIN_8BIT, pData, iNumBytes ) &&
       addData ( AP_CLIPBOARD_STRING, pData, iNumBytes ) &&
       addData ( AP_CLIPBOARD_TEXT_PLAIN, pData, iNumBytes ) &&
       addData ( AP_CLIPBOARD_COMPOUND_TEXT, pData, iNumBytes ) )
    return true ;
  return false ;
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
  return getData ( tFrom, aszFormatsAccepted, ppData, pLen, pszFormatFound ) ;
}

bool  AP_UnixClipboard::getTextData(T_AllowGet tFrom,
				    void ** ppData, UT_uint32 * pLen,
				    const char **pszFormatFound)
{
  return getData ( tFrom, txtszFormatsAccepted, ppData, pLen, pszFormatFound ) ;
}

bool  AP_UnixClipboard::getRichTextData(T_AllowGet tFrom,
					void ** ppData, UT_uint32 * pLen,
					const char **pszFormatFound)
{
  return getData ( tFrom, aszFormatsAccepted, ppData, pLen, pszFormatFound ) ;
}

bool AP_UnixClipboard::getImageData(T_AllowGet tFrom,
				    void ** ppData, UT_uint32 * pLen,
				    const char **pszFormatFound)
{
  return getData ( tFrom, imgszFormatsAccepted, ppData, pLen, pszFormatFound ) ;
}


bool AP_UnixClipboard::isTextTag ( const char * tag )
{
  if ( !strcmp ( tag, AP_CLIPBOARD_TEXTPLAIN_8BIT ) ||
       !strcmp ( tag, AP_CLIPBOARD_STRING ) ||
       !strcmp ( tag, AP_CLIPBOARD_TEXT_PLAIN ) ||
       !strcmp ( tag, AP_CLIPBOARD_COMPOUND_TEXT ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isRichTextTag ( const char * tag )
{
  if ( !strcmp ( tag, AP_CLIPBOARD_TXT_RTF ) ||
       !strcmp ( tag, AP_CLIPBOARD_APPLICATION_RTF ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isHTMLTag ( const char * tag )
{
  if ( !strcmp ( tag, AP_CLIPBOARD_TXT_HTML ) )
    return true ;
  return false ;
}

bool AP_UnixClipboard::isImageTag ( const char * tag )
{
  if ( !strncmp ( tag, "image/", 6 ) )
    return true ;
  return false ;
}
