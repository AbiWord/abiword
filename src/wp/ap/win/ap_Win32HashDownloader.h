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

#ifndef AP_WIN32HASHDOWNLOADER_H
#define AP_WIN32HASHDOWNLOADER_H


#include "ap_HashDownloader.h"


class ABI_EXPORT AP_Win32HashDownloader : public AP_HashDownloader
{
public:
	AP_Win32HashDownloader();
	~AP_Win32HashDownloader();

protected:
	virtual UT_sint32	execCommand(const char *szCommand);
	virtual tPkgType	wantedPackageType(XAP_Frame *pFrame);

	virtual UT_sint32 platformInstallPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType);
	virtual void showErrorMsg(XAP_Frame *pFrame, const char *errMsg, bool showErrno) const;

private:
};


#endif
