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

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "zlib.h"
#include "ispell_checker.h"

#include <time.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"

#include "xap_Frame.h"
#include "xap_DialogFactory.h"
#include "xap_Strings.h"

#include "ap_Dialog_Id.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"
#include "ap_HashDownloader.h"
#include "ap_Dialog_Download_File.h"

#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_path.h"
#include "ut_decompress.h"


AP_HashDownloader::AP_HashDownloader()
{
	abiSpellList = "abispell-list.%s.xml.gz";	// %s is replaced with endian specifier, eg. i386, PPC
	defaultAbiSpellListURL = "http://www.abisource.com/~fjf/%s";	// %s is complete abi spell list name
}

AP_HashDownloader::~AP_HashDownloader()
{
}

/*
 * return:	0  - Ok, no errors
 *			1  - user aborted download
 *			<0 - error
 */
UT_sint32
AP_HashDownloader::downloadFile(XAP_Frame *pFrame, UT_String szURL, const char *szDescription, XAP_HashDownloader::tFileData *d, UT_uint32 show_progress)
{
	const XAP_StringSet *pSS = pFrame->getApp()->getStringSet();
	XAP_DialogFactory * pDialogFactory = (XAP_DialogFactory *)(pFrame->getDialogFactory());
	AP_Dialog_Download_File *pDialog = (AP_Dialog_Download_File *)(pDialogFactory->requestDialog(AP_DIALOG_ID_DOWNLOAD_FILE));
	
	pDialog->setTitle(pSS->getValue(XAP_STRING_ID_DLG_HashDownloader_Dlg_Title));
	pDialog->setURL(szURL.c_str());
	pDialog->setDescription(szDescription);
	pDialog->runModal(pFrame);
	
	if (pDialog->getUserAnswer() == AP_Dialog_Download_File::a_CANCEL)
		return(1);
	
	if (!pDialog->getDLDone() || pDialog->getDLResult())
		return(-1);
	
	d->s = pDialog->getFileSize();
	d->data = (char *)malloc(d->s);
	pDialog->getFileData(d->data, 0, d->s);
	
	pDialogFactory->releaseDialog(pDialog);
	return(0);
}


const char * 
AP_HashDownloader::getAbiSpellListName(void)
{
	return abiSpellList;
}


const char * 
AP_HashDownloader::getDefaultAbiSpellListURL(void)
{
	//TODO: support loading an override from the preference file
	return defaultAbiSpellListURL;
}


UT_sint32
AP_HashDownloader::downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload)
{
	char *szPath;
	UT_sint32 ret, i;
	FILE *fp;
	gzFile gzfp;
	const XAP_StringSet *pSS = pFrame->getApp()->getStringSet();

#ifdef CURLHASH_NO_CACHING_OF_LIST
	forceDownload = 1;
#endif
        UT_String szURL, szFName;

	szFName = UT_String_sprintf (getAbiSpellListName(), endianess);
	szURL = UT_String_sprintf (getDefaultAbiSpellListURL(), szFName.c_str());
	szPath = UT_catPathname(XAP_App::getApp()->getUserPrivateDirectory(), szFName.c_str());
	UT_ASSERT((szPath) && (*szPath));

#ifdef CURLHASH_NEVER_UPDATE_LIST
	if (!UT_isRegularFile(szPath)) {
#else
	if (forceDownload || !UT_isRegularFile(szPath) || UT_mTime(szPath) + dictionaryListMaxAge < time(NULL)) {
#endif	
		if (fileData.data)
			free(fileData.data);
		fileData.data = NULL;
		fileData.s = 0;
		if ((ret = downloadFile(pFrame, szURL, pSS->getValue(XAP_STRING_ID_DLG_HashDownloader_DictList), &fileData, 0))) {
			if (ret < 0 && dlg_askFirstTryFailed(pFrame)) {
				if ((ret = downloadFile(pFrame, szURL, pSS->getValue(XAP_STRING_ID_DLG_HashDownloader_DictList), &fileData, 0))) {
					if (ret < 0) {
						showNoteDlg(pFrame, XAP_STRING_ID_DLG_HashDownloader_DictlistDLFail);
						return(-1);
					} else {
						return(1);
					}
				}
			} else
				return(1);
		}

		if (!(fp = fopen(szPath, "wb"))) {
			showErrorMsg(pFrame, "AP_HashDownloader::downloadDictionaryList(): fopen(, \"wb\") failed", true);
			return(-1);
		}
		fwrite(fileData.data, fileData.s, 1, fp);
		fclose(fp);
	}
	
	if (!(gzfp = gzopen(szPath, "rb"))) {
		showErrorMsg(pFrame, "AP_HashDownloader::downloadDictionaryList(): failed to open compressed dictionarylist", true);
		return(-1);
	}
	
	if (fileData.data)
		free(fileData.data);
	fileData.data = NULL;
	i = 0;
	/* Find out how many bytes the uncompressed file is */
	while ((ret = gzread(gzfp, const_cast<char *>(szURL.c_str()), 1)))
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
		UT_String errMsg;
		errMsg = UT_String_sprintf ("Error while parsing abispell dictionary-list (ret=%d - xmlParseOk=%d)\n", ret, xmlParseOk);
		showErrorMsg(pFrame, errMsg.c_str());
		return(-1);
	}

	return(0);
}


#ifdef CURLHASH_INSTALL_SYSTEMWIDE
UT_sint32
AP_HashDownloader::dlg_askInstallSystemwide(XAP_Frame *pFrame)
{
	if (XAP_Dialog_MessageBox::a_NO == pFrame->showMessageBox(XAP_STRING_ID_DLG_HashDownloader_AskInstallGlobal
				, XAP_Dialog_MessageBox::b_YN, XAP_Dialog_MessageBox::a_YES
				, pFrame->getApp()->getAbiSuiteLibDir()))
		return(0);

	return(1);
}
#endif


UT_sint32
AP_HashDownloader::installPackageUnsupported(XAP_Frame *pFrame, const char *szFName, const char *szLName, tPkgType pkgType)
{
	showNoteDlg(pFrame, "AP_XXXHashDownloader::installPackage(): package file support not implemented.\n");
	return (-1);
}


UT_sint32 
AP_HashDownloader::installPackage(XAP_Frame *pFrame, UT_String szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType, UT_sint32 rm)
{
	// we delegate the work to a platform specific function so we can properly clean up on failure 
	// and handle platform specific types such as RPM and maybe in the future EXEs
	UT_sint32 ret = platformInstallPackage(pFrame, szFName.c_str(), szLName, pkgType);

#ifdef DEBUG
	// Let the user know we succeeded
	if (!ret)
		showNoteDlg(pFrame, "Dictionary file successfully installed.\n");
#endif

	if (ret && (rm & RM_FAILURE))
		UT_unlink(szFName.c_str());

	if (!ret && (rm & RM_SUCCESS))
		UT_unlink(szFName.c_str());
	
	return (ret);
}


/*  Takes a already downloaded archive file and extracts the dictionary hash & encoding to proper location
 *  pFrame : useful for displaying dialog boxes or if other application data is needed
 *  szFName : the full path to the downloaded file (the archive with the dictionary that needs installed)
 *  szLName : the language for the dictionary we want to install
 *  pkgType : format of the archive, currently only tarballs supported, zip planned, RPM never
 *  rm : NOT USED, specifies whether to delete the file specified by szFName or not [set of flags determine case]
 */
UT_sint32
AP_HashDownloader::platformInstallPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType)
{
  const char *name = NULL, *hname;
  UT_String pName;
  int ret;
  
  if (pkgType == pkgType_Tarball) 
    {
#ifdef CURLHASH_INSTALL_SYSTEMWIDE
      if (getInstallSystemWide())
	pName = XAP_App::getApp()->getAbiSuiteLibDir();
      else
#endif
	pName = XAP_App::getApp()->getUserPrivateDirectory();
      
      // try to make sure the base path exists (e.g. while testing Debug builds)
      XAP_App::getApp()->makeDirectory(pName.c_str(), 0750);
      if (!UT_directoryExists(pName.c_str())) 
	{
	  showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Error base abisuite directory does not exist\n");
	  return (-1);
	}
      
      pName += "/dictionary";
      //name = UT_strdup(pName.c_str());
      name = pName.c_str();
      
      UT_sint32 langNdx;
      if ((langNdx = getLangNum(szLName)) == -1) 
	{
	  showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): No matching hashname to that language\n");
	  return (-1);
	}
      hname = m_mapping[langNdx].dict;
      
      UT_DEBUGMSG(("AP_XXXHashDownloader::installPackage(): extracting %s to %s\n",hname, name));
      
      // ensure the dictionary directory exists
      XAP_App::getApp()->makeDirectory(name, 0750);
      if (!UT_directoryExists(name)) 
	{
	  showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Error while creating dictionary-directory\n");
	  return (-1);
	}
      
      // actually extracts the hash file
      if ((ret = UT_untgz(szFName, hname, name, NULL, NULL))) {
	showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Error while extracting hash\n");
	return(ret);
      }
      
      // ...and the -encoding file
      UT_String enc(hname);
      enc += "-encoding";
      if ((ret = UT_untgz(szFName, enc.c_str(), name, NULL, NULL))) {
	showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Error while extracting hash-encoding\n");
	return(ret);
      }
    }
  else
    {	
      return installPackageUnsupported(pFrame, szFName, szLName, pkgType);
    }
}

