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
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "zlib.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_rand.h"

#include "xap_App.h"
#include "xap_Frame.h"

#include "ap_QNXHashDownloader.h"

#include "ispell_checker.h"


extern char **environ;


AP_QNXHashDownloader::AP_QNXHashDownloader()
{
}

AP_QNXHashDownloader::~AP_QNXHashDownloader()
{
}


void AP_QNXHashDownloader::showErrorMsg(XAP_Frame *pFrame, const char *errMsg, bool showErrno) const
{
	if (showErrno)
		perror(errMsg);
	else
		fprintf(stderr, "%s", errMsg);
}

UT_sint32
AP_QNXHashDownloader::isRoot()
{
	if (geteuid() == 0)
		return(1);

	return(0);
}


UT_sint32
AP_QNXHashDownloader::execCommand(const char *szCommand)
{
	int pid, status;
	char *argv[10];

	if (!szCommand)
		return -1;
		
	if ((pid = fork()) == -1)
		return -1;
		
	if (pid == 0) {
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = (char *)szCommand;
		argv[3] = 0;
		execve("/bin/sh", argv, environ);
		exit(127);
	}
	do {
		if (waitpid(pid, &status, 0) == -1) {
			if (errno != EINTR)
				return -1;
		} else
			return status;
	} while(1);
}


XAP_HashDownloader::tPkgType 
AP_QNXHashDownloader::wantedPackageType(XAP_Frame *pFrame)
{
#ifdef CURLHASH_INSTALL_SYSTEMWIDE
	if (isRoot() && dlg_askInstallSystemwide(pFrame)) {
		setInstallSystemWide(true);
	}
	else
		setInstallSystemWide(false);
#endif

#if defined(CURLHASH_USE_RPM) && defined(CURLHASH_INSTALL_SYSTEMWIDE)
	return(pkgType_RPM);
#else
	return(pkgType_Tarball);
#endif
}


UT_sint32 
AP_QNXHashDownloader::platformInstallPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType)
{
	char buff[512];
	UT_sint32 ret;
	UT_String pName;
	
	if (pkgType == pkgType_RPM) {
		sprintf(buff, "rpm -Uvh %s", szFName);
		ret = execCommand(buff);
	}

#if 1
	else
		ret = AP_HashDownloader::platformInstallPackage(pFrame, szFName, szLName, pkgType);

#else

	if (pkgType == pkgType_Tarball) {
#ifdef CURLHASH_INSTALL_SYSTEMWIDE
		if (getInstallSystemWide())
			pName = XAP_App::getApp()->getAbiSuiteLibDir();
		else
#endif
			pName = XAP_App::getApp()->getUserPrivateDirectory();

		pName += "/dictionary/";
		name = UT_strdup(pName.c_str());
		
		if ((ret = getLangNum(szLName)) == -1) {
			fprintf(stderr, "AP_QNXHashDownloader::installPackage(): No matching hashname to that language\n");
			return(-1);
		}
		hname = m_mapping[ret].dict;
		
		ret = UT_rand();
		sprintf(tmpDir, "/tmp/abiword_abispell-install-%d/", ret);
		
		sprintf(buff, "mkdir -p %s", tmpDir);
		if ((ret = execCommand(buff))) {
			fprintf(stderr, "AP_QNXHashDownloader::installPackage(): Error while making tmp directory\n");
			return(ret);
		}
		
		sprintf(buff, "cd %s; gzip -dc %s | tar xf -", tmpDir, szFName);
		if ((ret = execCommand(buff))) {
			fprintf(stderr, "AP_QNXHashDownloader::installPackage(): Error while unpacking the tarball\n");
			return(ret);
		}

		sprintf(buff, "mkdir -p %s", name);
		if ((ret = execCommand(buff))) {
			fprintf(stderr, "AP_QNXHashDownloader::installPackage(): Error while creating dictionary-directory\n");
			return(ret);
		}
		
		/*
		 * Handles packages that either have just the files, or that also
		 * contain the whole path usr/shate/AbiSuite/dictionary/
		 */
		sprintf(buff, "cd %susr/share/AbiSuite/dictionary/; mv %s %s", tmpDir, hname, name);
		if ((ret = execCommand(buff))) {
			sprintf(buff, "cd %s; mv %s %s", tmpDir, hname, name);
			if ((ret = execCommand(buff))) {
				fprintf(stderr, "AP_QNXHashDownloader::installPackage(): Error while moving dictionary into place\n");
				return(ret);
			}
		}


		sprintf(buff, "rm -rf %s", tmpDir);
		if ((ret = execCommand(buff))) {
			fprintf(stderr, "AP_QNXHashDownloader::installPackage(): Error while removing tmp directory\n");
			return(ret);
		}
	}
#endif

	return(ret);
}
