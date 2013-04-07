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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_UNIXCLIPBOARD_H
#define AP_UNIXCLIPBOARD_H

#include "xap_UnixClipboard.h"
class AP_UnixApp;

class AP_UnixClipboard : public XAP_UnixClipboard
{
public:
	AP_UnixClipboard(AP_UnixApp * pUnixApp);

	bool addTextData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes);
	bool addRichTextData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes);
	bool addHtmlData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes, bool xhtml);
	bool addODTData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes);
	bool addPNGData(T_AllowGet tTo, const void* pData, UT_sint32 iNumBytes);

	bool getSupportedData(T_AllowGet tFrom,
			      const void ** ppData, UT_uint32 * pLen,
			      const char **pszFormatFound);

	bool  getTextData(T_AllowGet tFrom,
					  const void ** ppData, UT_uint32 * pLen,
					  const char **pszFormatFound);

	bool  getRichTextData(T_AllowGet tFrom,
			      const void ** ppData, UT_uint32 * pLen,
			      const char **pszFormatFound);

	bool getImageData(T_AllowGet tFrom,
			  const void ** ppData, UT_uint32 * pLen,
			  const char **pszFormatFound);

	bool getDynamicData(T_AllowGet tFrom,
			  const void ** ppData, UT_uint32 * pLen,
			  const char **pszFormatFound);

	void addFormat(const char * fmt);
	void deleteFormat(const char * fmt);

	static bool isTextTag ( const char * tag ) ;
	static bool isRichTextTag ( const char * tag ) ;
	static bool isHTMLTag ( const char * tag ) ;
	static bool isImageTag ( const char * tag ) ;
	static bool isDynamicTag ( const char * tag ) ;
};

#endif /* AP_UNIXCLIPBOARD_H */
