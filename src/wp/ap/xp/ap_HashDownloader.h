/* AbiWord
 * Copyright (C) 2002 Gabriel
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

#ifndef AP_HASHDOWNLOADER_H
#define AP_HASHDOWNLOADER_H


#include <curl/types.h>
#include "xap_HashDownloader.h"


class ABI_EXPORT AP_HashDownloader : public XAP_HashDownloader
{
public:
	AP_HashDownloader();
	~AP_HashDownloader();

protected:
	virtual void		showProgressStart(XAP_Frame *pFrame, CURL *ch);
	virtual	void		showProgressStop(XAP_Frame *pFrame, CURL *ch);

	virtual UT_sint32	downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload) = 0;
	virtual tPkgType	wantedPackageType(XAP_Frame *pFrame) = 0;
	virtual UT_sint32	installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, tPkgType pkgType, UT_sint32 rm) = 0;
	
private:
	bool didShowStatusBar;
};


#endif
