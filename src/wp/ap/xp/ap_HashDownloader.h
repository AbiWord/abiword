/* AbiWord
 * Copyright (C) 2002 Gabriel Gerhardsson
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


/* 
 * WARNING!
 * Do not enable this unless you know what you do!
 * It works, but don't come screaming if Abiword destroyed your harddrive while
 * you where running it as root
 */
/*#define CURLHASH_INSTALL_SYSTEMWIDE */



class ABI_EXPORT AP_HashDownloader : public XAP_HashDownloader
{
public:
	AP_HashDownloader();
	~AP_HashDownloader();

protected:
	virtual void 		showErrorMsg(XAP_Frame *pFrame, const char *errMsg, bool showErrno=false) const = 0;

	virtual tPkgType	wantedPackageType(XAP_Frame *pFrame) = 0;

	virtual UT_sint32 	downloadFile(XAP_Frame *pFrame, UT_String szURL, const char *szDescription, XAP_HashDownloader::tFileData *d, UT_uint32 show_progress);
	virtual UT_sint32	downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload);

	// calls platformInstallPackage, based on result and rm flag handles removal of downloaded package
	virtual UT_sint32	installPackage(XAP_Frame *pFrame, UT_String szFName, const char *szLName, tPkgType pkgType, UT_sint32 rm);
	// displays message to user indicated downloaded package format unsupported
	virtual UT_sint32	installPackageUnsupported(XAP_Frame *pFrame, const char *szFName, const char *szLName, tPkgType pkgType);

	// by default platformInstallPackage only supports tarballs and call installPackage Unsupported for all
	// others, a subclass should override it to support specific others and optionally calling this one to handle the rest
	virtual UT_sint32 platformInstallPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType);
	
	virtual const char * getAbiSpellListName(void);
	virtual const char * getDefaultAbiSpellListURL(void);

#ifdef CURLHASH_INSTALL_SYSTEMWIDE
	virtual UT_sint32	dlg_askInstallSystemwide(XAP_Frame *pFrame);
	virtual void 		setInstallSystemWide(bool isw) { _installSystemwide = isw; }
	virtual bool 		getInstallSystemWide(void) { return _installSystemwide; }
#endif

private:
	bool didShowStatusBar;

#ifdef CURLHASH_INSTALL_SYSTEMWIDE
	bool _installSystemwide;
#endif

	const char * abiSpellList;
	const char * defaultAbiSpellListURL;

	UT_sint32	_untgz(XAP_Frame *pFrame, const char *szFName, const char *hashname, const char *destpath);
};


#endif
