/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include "zlib.h"

#include "xap_App.h"
#include "ut_string_class.h"

#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_rand.h"

#include "xap_Frame.h"
#include "ap_UnixHashDownloader.h"

#include "ispell_checker.h"


AP_UnixHashDownloader::AP_UnixHashDownloader()
{
}

AP_UnixHashDownloader::~AP_UnixHashDownloader()
{
}


UT_sint32
AP_UnixHashDownloader::downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload)
{
	char szURL[256], szFName[128], szPath[512];
	UT_sint32 ret, i;
	FILE *fp;
	gzFile gzfp;
	struct stat statBuf;
	
	sprintf(szFName, "abispell-list.%s.xml.gz", endianess);
	sprintf(szURL, "http://www.abisource.com/~fjf/%s", szFName);
	sprintf(szPath, "%s/%s", XAP_App::getApp()->getUserPrivateDirectory(), szFName);
	

	if (forceDownload || stat(szPath, &statBuf) || statBuf.st_mtime + dictionaryListMaxAge < time(NULL)) {
		if (fileData.data)
			free(fileData.data);
		fileData.data = NULL;
		if ((ret = downloadFile(pFrame, szURL, &fileData, 0))) {
			if (dlg_askFirstTryFailed(pFrame))
				if ((ret = downloadFile(pFrame, szURL, &fileData, 0))) {
					pFrame->showMessageBox("Download of the dictionary-list failed!\n"
							, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
					return(-1);
				}
		}

		if (!(fp = fopen(szPath, "wb"))) {
			perror("AP_UnixHashDownloader::downloadDictionaryList(): fopen(, \"wb\") failed");
			return(-1);
		}
		fwrite(fileData.data, fileData.s, 1, fp);
		fclose(fp);
	}
	
	
	if (!(gzfp = gzopen(szPath, "rb"))) {
		perror("downloadDictionaryList(): failed to open compressed dictionarylist");
		return(-1);
	}
	
	if (fileData.data)
		free(fileData.data);
	fileData.data = NULL;
	i = 0;
	/* Find out how many bytes the uncompressed file is */
	while ((ret = gzread(gzfp, szURL, 1)))
		i += ret;

	fileData.data = (char *)malloc(i);
	fileData.s = i;
	gzrewind(gzfp);
	/* Read for real */
	gzread(gzfp, fileData.data, i);
	gzclose(gzfp);
	
	/* Parse XML */
	initData();
	xmlParser.setListener(this);
	if ((ret = xmlParser.parse(fileData.data, fileData.s)) || xmlParseOk == -1) {
		fprintf(stderr, "Error while parsing abispell dictionary-list (ret=%d - xmlParseOk=%d\n", ret, xmlParseOk);
		return(-1);
	}

	return(0);
}


UT_sint32
AP_UnixHashDownloader::dlg_askInstallSystemwide(XAP_Frame *pFrame)
{
	if (XAP_Dialog_MessageBox::a_NO == pFrame->showMessageBox(
				"You seem to have permission to install the dictionary system-wide.\n"
				"Would you like to do that?"
				, XAP_Dialog_MessageBox::b_YN, XAP_Dialog_MessageBox::a_YES))
		return(0);

	return(1);
}


UT_sint32
AP_UnixHashDownloader::isRoot()
{
	if (geteuid() == 0)
		return(1);

	return(0);
}


XAP_HashDownloader::tPkgType 
AP_UnixHashDownloader::wantedPackageType(XAP_Frame *pFrame)
{
	installSystemwide = 0;
	
#ifdef LET_ROOT_INSTALL_SYSTEMWIDE
#ifdef IS_RUNNING_ON_RPM_SYSTEM
	if (isRoot() && dlg_askInstallSystemwide(pFrame)) {
		installSystemwide = 1;
		return(pkgType_RPM);
	}
#endif /* IS_RUNNING_ON_RPM_SYSTEM */

	if (isRoot() && dlg_askInstallSystemwide(pFrame))
		installSystemwide = 1;
#endif

	return(pkgType_Tarball);
}


UT_sint32 
AP_UnixHashDownloader::installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType, UT_sint32 rm)
{
	char buff[512], tmpDir[512];
	UT_sint32 ret;
        char *name = NULL, *hname;
	UT_String pName;
	
	if (pkgType == pkgType_RPM) {
		sprintf(buff, "rpm -Uvh %s", szFName);
		ret = mySystem(buff);
	}


	if (pkgType == pkgType_Tarball) {
		if (installSystemwide)
			pName = XAP_App::getApp()->getAbiSuiteLibDir();
		else
			pName = XAP_App::getApp()->getUserPrivateDirectory();

		pName += "/dictionary/";
		name = UT_strdup(pName.c_str());
		
		if ((ret = getLangNum(szLName)) == -1) {
			fprintf(stderr, "AP_UnixHashDownloader::installPackage(): No matching hashname to that language\n");
			return(-1);
		}
		hname = m_mapping[ret].dict;
		
		ret = UT_rand();
		sprintf(tmpDir, "/tmp/abiword_abispell-install-%d/", ret);
		
		sprintf(buff, "mkdir -p %s", tmpDir);
		if ((ret = mySystem(buff))) {
			fprintf(stderr, "AP_UnixHashDownloader::installPackage(): Error while making tmp directory\n");
			return(ret);
		}
		
		sprintf(buff, "cd %s; gzip -dc %s | tar xf -", tmpDir, szFName);
		if ((ret = mySystem(buff))) {
			fprintf(stderr, "AP_UnixHashDownloader::installPackage(): Error while unpacking the tarball\n");
			return(ret);
		}

		sprintf(buff, "mkdir -p %s", name);
		if ((ret = mySystem(buff))) {
			fprintf(stderr, "AP_UnixHashDownloader::installPackage(): Error while creating dictionary-directory\n");
			return(ret);
		}
		
		sprintf(buff, "cd %susr/share/AbiSuite/dictionary/; mv %s %s-encoding %s", tmpDir, hname, hname, name);
		if ((ret = mySystem(buff))) {
			fprintf(stderr, "AP_UnixHashDownloader::installPackage(): Error while moving dictionary into place\n");
			return(ret);
		}

		sprintf(buff, "rm -rf %s", tmpDir);
		if ((ret = mySystem(buff))) {
			fprintf(stderr, "AP_UnixHashDownloader::installPackage(): Error while removing tmp directory\n");
			return(ret);
		}
	}

	if (ret && (rm & RM_FAILURE))
		remove(szFName);

	if (!ret && (rm & RM_SUCCESS))
		remove(szFName);
	
	return(ret);
}
