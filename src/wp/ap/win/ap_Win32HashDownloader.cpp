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

#include <stdio.h>

#include "xap_App.h"
#include "xap_Frame.h"
#include "ap_Win32HashDownloader.h"

AP_Win32HashDownloader::AP_Win32HashDownloader()
{
}

AP_Win32HashDownloader::~AP_Win32HashDownloader()
{
}


// This can be empty as we never use it (though subclasses may)
UT_sint32
AP_Win32HashDownloader::execCommand(const char *szCommand)
{
	system(szCommand);
	return 0;
}


XAP_HashDownloader::tPkgType 
AP_Win32HashDownloader::wantedPackageType(XAP_Frame *pFrame)
{
#ifdef CURLHASH_INSTALL_SYSTEMWIDE
	if (dlg_askInstallSystemwide(pFrame))
		setInstallSystemWide(true);
	else
		setInstallSystemWide(false);
#endif

// #ifdef CURLHASH_PREFER_TARBALL
#if 1
	return(pkgType_Tarball);
#else
	return(pkgType_Zip);
#endif
}


void AP_Win32HashDownloader::showErrorMsg(XAP_Frame *pFrame, const char *errMsg, bool showErrno) const
{
	if (showErrno)
	{
		UT_String msg(errMsg);
		msg += ": ";
		msg += strerror(errno);
		pFrame->showMessageBox(msg.c_str(), XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
	}
	else
	{
		pFrame->showMessageBox(errMsg, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
	}
}


UT_sint32
AP_Win32HashDownloader::platformInstallPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType)
{
	return AP_HashDownloader::platformInstallPackage(pFrame, szFName, szLName, pkgType);
}
