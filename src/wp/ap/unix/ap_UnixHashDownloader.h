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

#ifndef AP_UNIXHASHDOWNLOADER_H
#define AP_UNIXHASHDOWNLOADER_H


#include "ap_HashDownloader.h"


/* 
 * WARNING!
 * Do not enable this unless you know what you do!
 * It works, but don't come screaming if Abiword destroyed your harddrive while
 * you where running it as root
 */
/*#define CURLHASH_INSTALL_SYSTEMWIDE */

/*
 * If you've enabled the above option and are running a rpm system, you can
 * enable this. If current user is root it will try to install an rpm instead
 * of a tar.gz
 */
/*#define CURLHASH_USE_RPM */


class ABI_EXPORT AP_UnixHashDownloader : public AP_HashDownloader
{
public:
	AP_UnixHashDownloader();
	~AP_UnixHashDownloader();

protected:
	virtual UT_sint32	dlg_askInstallSystemwide(XAP_Frame *pFrame);
	virtual UT_sint32	isRoot(void);
	
	virtual UT_sint32	execCommand(const char *szCommand);
	virtual UT_sint32	downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload);
	virtual tPkgType	wantedPackageType(XAP_Frame *pFrame);
	virtual UT_sint32	installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, tPkgType pkgType, UT_sint32 rm);
	
private:
	UT_sint32		installSystemwide;
};


#endif
