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

#include <time.h>
#include <sys/stat.h>
#include <direct.h>

#include "zlib.h"

#include "ut_debugmsg.h"
#include "ut_path.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "ap_Win32HashDownloader.h"

#include "ispell_checker.h"


AP_Win32HashDownloader::AP_Win32HashDownloader()
{
}

AP_Win32HashDownloader::~AP_Win32HashDownloader()
{
}


UT_sint32
AP_Win32HashDownloader::downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload)
{
	char szURL[256], szFName[128], szPath[512];
	UT_sint32 ret, i;
	FILE *fp;
	gzFile gzfp;
	struct stat statBuf;

#ifdef CURLHASH_NO_CACHING_OF_LIST
	forceDownload = 1;
#endif

	sprintf(szFName, "abispell-list.%s.xml.gz", endianess);
	sprintf(szURL, "http://www.abisource.com/~fjf/%s", szFName);
	sprintf(szPath, "%s/%s", XAP_App::getApp()->getUserPrivateDirectory(), szFName);
	

#ifdef CURLHASH_NEVER_UPDATE_LIST
	if (stat(szPath, &statBuf)) {
#else
	if (forceDownload || stat(szPath, &statBuf) || statBuf.st_mtime + dictionaryListMaxAge < time(NULL)) {
#endif	
		if (fileData.data)
			free(fileData.data);
		fileData.data = NULL;
		if ((ret = downloadFile(pFrame, szURL, &fileData, 0))) {
			if (dlg_askFirstTryFailed(pFrame))
				if ((ret = downloadFile(pFrame, szURL, &fileData, 0))) {
					pFrame->showMessageBox(XAP_STRING_ID_DLG_HashDownloader_DictlistDLFail,
						XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK, NULL);
					return(-1);
				}
		}

		if (!(fp = fopen(szPath, "wb"))) {
			pFrame->showMessageBox("AP_Win32HashDownloader::downloadDictionaryList(): fopen(, \"wb\") failed",
				XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
			return(-1);
		}
		fwrite(fileData.data, fileData.s, 1, fp);
		fclose(fp);
	}
	
	if (!(gzfp = gzopen(szPath, "rb"))) {
		pFrame->showMessageBox("downloadDictionaryList(): failed to open compressed dictionarylist",
				XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
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
		char buf[1024];
		sprintf(buf, "Error while parsing abispell dictionary-list (ret=%d - xmlParseOk=%d\n", ret, xmlParseOk);
		pFrame->showMessageBox(buf, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return(-1);
	}

	return(0);
}


UT_sint32
AP_Win32HashDownloader::dlg_askInstallSystemwide(XAP_Frame *pFrame)
{
	if (XAP_Dialog_MessageBox::a_NO == pFrame->showMessageBox(XAP_STRING_ID_DLG_HashDownloader_AskInstallGlobal
				, XAP_Dialog_MessageBox::b_YN, XAP_Dialog_MessageBox::a_YES
				, pFrame->getApp()->getAbiSuiteLibDir()))
		return(0);

	return(1);
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
		installSystemwide = 1;
	else
#endif
	installSystemwide = 0;

// #ifdef CURLHASH_PREFER_TARBALL
#if 1
	return(pkgType_Tarball);
#else
	return(pkgType_Zip);
#endif
}


void AP_Win32HashDownloader::showErrorMsg(XAP_Frame *pFrame, const char *errMsg) const
{
	pFrame->showMessageBox(errMsg, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
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
	char * fn;

	// get the last path separator in fname (fn will be NULL if there are none)
	fn = strrchr(fname, '/');
#ifdef _WIN32
	{
		char * fn2 = strrchr((fn == NULL)?fname:fn, '\\');
		if (fn2 != NULL) fn = fn2;
	}
#endif

	if (fn != NULL)
	{
		// be sure terminating '\0' is copied and
		// use ansi memcpy equivalent that handles overlapping regions
		memmove(fname, fn+1, strlen(fn) /* strlen(fn+1) + 1 */);
	}

	return fname;
}


/* return 0 on success
 * extracts only the specified dictionary and its encoding (other files and paths ignored)
 * note that destpath lacks slash at end
 */
UT_sint32 AP_Win32HashDownloader::_untgz(XAP_Frame *pFrame, gzFile tarball, const char *hashname, const char *destpath)
{
	union  tar_buffer buffer;
	int    getheader = 1;
	int    remaining = 0;
	int    len;
	char   fname[BLOCKSIZE];
	FILE   *outfile = NULL;

	bool done = false;
	while (!done)
	{
		if ((len = gzread(tarball, &buffer, BLOCKSIZE)) != BLOCKSIZE)
		{
			// error (gzerror(in, &err));
			showErrorMsg(pFrame, "untgz: gzread failed to read in complete block");
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
			strcpy(fname,buffer.header.name);
			strippath(fname);
	  
			if ((buffer.header.typeflag == '\0')	||	// [A]REGTYPE, ie regular files
				(buffer.header.typeflag == '0') )
			{
				remaining = getoct(buffer.header.size,12);

				if ((remaining) && (matchname(hashname,fname)))
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
AP_Win32HashDownloader::_installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType, UT_sint32 rm)
{
	const char *name = NULL, *hname;
	UT_String pName;
	UT_sint32 ret;

	if (pkgType == pkgType_Zip) 
	{
		pFrame->showMessageBox("AP_XXXHashDownloader::installPackage(): Zip file support not implemented yet!\n",
			XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
			return (-1);
	} 
	else if (pkgType == pkgType_Tarball) 
	{
		if (installSystemwide)
			pName = XAP_App::getApp()->getAbiSuiteLibDir();
		else
			pName = XAP_App::getApp()->getUserPrivateDirectory();

		pName += "/dictionary";
		//name = UT_strdup(pName.c_str());
		name = pName.c_str();
		
		if ((ret = getLangNum(szLName)) == -1) 
		{
			showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): No matching hashname to that language\n");
			return (-1);
		}
		hname = m_mapping[ret].dict;

		UT_DEBUGMSG(("AP_XXXHashDownloader::installPackage(): extracting %s to %s\n",hname, name));

		// ensure the dictionary directory exists
		// NOTE: this assumes the base path exists (->getAbiSuiteLibDir() or ->getUserPrivateDirectory())
		_mkdir(name);
		if (!UT_directoryExists(name)) {
			showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Error while creating dictionary-directory\n");
			return (-1);
		}

		// szFName is package that was downloaded
		// extract files in tarball to appropriate directory, ignoring any paths in archive
		// extraction routines derived from logic in zlib's contrib untgz.c program
		gzFile tgzf = gzopen(szFName, "rb");
		if (tgzf == NULL)
		{
			showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Error while opening downloaded dictionary archive\n");
			return (-1);
		}
		// actually extracts the hash file and encoding hname and hname-encoding
		ret = _untgz(pFrame, tgzf, hname, name);
		gzclose(tgzf);
		return ret;
	}
	else	// if ( (pkgType == pkgType_RPM) || (pkgType == pkgType_NONE) )
	{
		showErrorMsg(pFrame, "AP_XXXHashDownloader::installPackage(): Specified package type not supported on Windows!\n");
		return (-1);
	}
}

UT_sint32 
AP_Win32HashDownloader::installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, XAP_HashDownloader::tPkgType pkgType, UT_sint32 rm)
{
	// we delegate the work to a private function so we can properly clean up on failure
	UT_sint32 ret = _installPackage(pFrame, szFName, szLName, pkgType, rm);

#ifdef DEBUG
	// Let the user know we succeeded
	if (!ret)
	{
		pFrame->showMessageBox("Dictionary file successfully installed.\n",
			XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
	}
#endif

	if (ret && (rm & RM_FAILURE))
		UT_unlink(szFName);

	if (!ret && (rm & RM_SUCCESS))
		UT_unlink(szFName);
	
	return(ret);
}
