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
#include "ut_path.h"

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
AP_HashDownloader::downloadFile(XAP_Frame *pFrame, const char *szURL, const char *szDescription, XAP_HashDownloader::tFileData *d, UT_uint32 show_progress)
{
	const XAP_StringSet *pSS = pFrame->getApp()->getStringSet();
	XAP_DialogFactory * pDialogFactory = (XAP_DialogFactory *)(pFrame->getDialogFactory());
	AP_Dialog_Download_File *pDialog = (AP_Dialog_Download_File *)(pDialogFactory->requestDialog(AP_DIALOG_ID_DOWNLOAD_FILE));
	
	pDialog->setTitle(pSS->getValue(XAP_STRING_ID_DLG_HashDownloader_Dlg_Title));
	pDialog->setURL(szURL);
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
	char szURL[256], szFName[128], *szPath;
	UT_sint32 ret, i;
	FILE *fp;
	gzFile gzfp;
	const XAP_StringSet *pSS = pFrame->getApp()->getStringSet();

#ifdef CURLHASH_NO_CACHING_OF_LIST
	forceDownload = 1;
#endif

	snprintf(szFName, sizeof(szFName), getAbiSpellListName(), endianess);
	snprintf(szURL, sizeof(szURL), getDefaultAbiSpellListURL(), szFName);
	szPath = UT_catPathname(XAP_App::getApp()->getUserPrivateDirectory(), szFName);
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
		char errMsg[1024];
		snprintf(errMsg, sizeof(errMsg), "Error while parsing abispell dictionary-list (ret=%d - xmlParseOk=%d)\n", ret, xmlParseOk);
		showErrorMsg(pFrame, errMsg);
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
AP_HashDownloader::installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType, UT_sint32 rm)
{
	// we delegate the work to a platform specific function so we can properly clean up on failure 
	// and handle platform specific types such as RPM and maybe in the future EXEs
	UT_sint32 ret = platformInstallPackage(pFrame, szFName, szLName, pkgType);

#ifdef DEBUG
	// Let the user know we succeeded
	if (!ret)
		showNoteDlg(pFrame, "Dictionary file successfully installed.\n");
#endif

	if (ret && (rm & RM_FAILURE))
		UT_unlink(szFName);

	if (!ret && (rm & RM_SUCCESS))
		UT_unlink(szFName);
	
	return (ret);
}


/* Portions based on or from untgz.c in zlib contrib directory */
#define BLOCKSIZE 512

struct tar_header
{				/* byte offset */
  char name[100];		/*   0 */
  char mode[8];			/* 100 */
  char uid[8];			/* 108 */
  char gid[8];			/* 116 */
  char size[12];		/* 124 */
  char mtime[12];		/* 136 */
  char chksum[8];		/* 148 */
  char typeflag;		/* 156 */
  char linkname[100];		/* 157 */
  char magic[6];		/* 257 */
  char version[2];		/* 263 */
  char uname[32];		/* 265 */
  char gname[32];		/* 297 */
  char devmajor[8];		/* 329 */
  char devminor[8];		/* 337 */
  char prefix[155];		/* 345 */
				/* 500 */
};

union tar_buffer {
  char               buffer[BLOCKSIZE];
  struct tar_header  header;
};


static int getoct(char *p,int width)
{
  int result = 0;
  char c;
  
  while (width --)
    {
      c = *p++;
      if (c == ' ')
	continue;
      if (c == 0)
	break;
      result = result * 8 + (c - '0');
    }
  return result;
}


static bool matchname (const char *hashname, const char *fname)
{
	UT_String enc(hashname);
	enc += "-encoding";

	if ((UT_stricmp(fname, hashname) == 0) || 
		(UT_stricmp(fname, enc.c_str()) == 0))
		return true;
	else
		return false;
}


// we strip any path components
static char * strippath(char *fname)
{
	const char * fn = UT_basename(fname);

	// be sure terminating '\0' is copied and
	// use ansi memcpy equivalent that handles overlapping regions
	memmove(fname, fn, strlen(fn) + 1 );

	return fname;
}


/* return 0 on success
 * extract files in tarball to appropriate directory, ignoring any paths in archive
 * extracts only the specified dictionary and its encoding (other files and paths ignored)
 * note that destpath lacks slash at end
 * szFName is package that was downloaded
 *
 * extraction routines derived from logic in zlib's contrib untgz.c program
 */
UT_sint32 AP_HashDownloader::_untgz(XAP_Frame *pFrame, const char *szFName, const char *hashname, const char *destpath)
{
	gzFile tarball;
	union  tar_buffer buffer;
	int    getheader = 1;
	int    remaining = 0;
	int    len;
	char   fname[BLOCKSIZE];
	FILE   *outfile = NULL;


	if ((tarball = gzopen(szFName, "rb")) == NULL)
	{
		showErrorMsg(pFrame, "untgz: Error while opening downloaded dictionary archive\n");
		return 1;
	}


	bool done = false;
	while (!done)
	{
		if ((len = gzread(tarball, &buffer, BLOCKSIZE)) != BLOCKSIZE)
		{
			// error (gzerror(in, &err));
			showErrorMsg(pFrame, "untgz: gzread failed to read in complete block");
			gzclose(tarball);
			return 1;
		}
		
		/*
		 * If we have to get a tar header
		 */
		if (getheader == 1)
		{
			/*
			 * if we met the end of the tar
			 * or the end-of-tar block,
			 * we are done
			 */
			if ((len == 0)  || (buffer.header.name[0]== 0)) 
			{ 
				done = true;
				continue; 
			}

			// tartime = (time_t)getoct(buffer.header.mtime,12);
			strcpy(fname, buffer.header.name);
			strippath(fname);
	  
			if ((buffer.header.typeflag == '\0')	||	// [A]REGTYPE, ie regular files
				(buffer.header.typeflag == '0') )
			{
				remaining = getoct(buffer.header.size, 12);

				if ((remaining) && (matchname(hashname, fname)))
				{
					UT_String outfilename(destpath);
					outfilename += "/";
					outfilename += fname;
					if ((outfile = fopen(outfilename.c_str(), "wb")) == NULL)
					{
						UT_String errMsg("untgz: Unable to extract ");
						errMsg += outfilename;
						showErrorMsg(pFrame, errMsg.c_str());
					}
				}
				else
					outfile = NULL;

				/*
				 * could have no contents
				 */
				getheader = (remaining) ? 0 : 1;
			}
		}
		else // if (getheader != 1)
		{
			unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;

			if (outfile != NULL)
			{
				if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
				{
					UT_String errMsg("untgz: error writing, skipping ");
					errMsg += fname;
					showErrorMsg(pFrame, errMsg.c_str());
					fclose(outfile);
					UT_unlink(fname);
				}
			}
			remaining -= bytes;
			if (remaining == 0)
			{
				getheader = 1;
				if (outfile != NULL)
				{
					// TODO: should actually set proper time from archive, oh well
					fclose(outfile);
					outfile = NULL;
				}
			}
		} // if (getheader == 1) else end
	}

	if (tarball != NULL) gzclose(tarball);
	return 0;
}
/* End from untgz.c */


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

		// actually extracts the hash file and encoding hname and hname-encoding
		return _untgz(pFrame, szFName, hname, name);
	}
	else	
		return installPackageUnsupported(pFrame, szFName, szLName, pkgType);
}

