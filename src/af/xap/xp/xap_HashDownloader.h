/* AbiSource Application Framework
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

#ifndef XAP_HASHDOWNLOADER_H
#define XAP_HASHDOWNLOADER_H


#include <curl/types.h>
#include <time.h>		// for time_t


/*
 * If there exists a local copy of the dictionary-list, never downlaod a 
 * updated version.
 * Overrides CURLHASH_NO_CACHING_OF_LIST
 */
/*#define CURLHASH_NEVER_UPDATE_LIST */

/*
 * Always download a updated version of the dictionary-list, no matter how new
 * the existing one is.
 */
/*#define CURLHASH_NO_CACHING_OF_LIST */

/*
 * Maximum allowed age of dictionary-list before a updated version is to be
 * downloaded. In seconds.
 */
#define CURLHASH_LISTCACHE_MAX_AGE 60*60*1

#define MAX_NUM_LANGUAGES 100


class ABI_EXPORT XAP_HashDownloader : public UT_XML::Listener
{
public:
	XAP_HashDownloader();
	~XAP_HashDownloader();

	UT_sint32 suggestDownload(XAP_Frame *f, const char *szLang);
	

	/* When to remove dictionary package */
	typedef enum {
		RM_SUCCESS 	= 0x1,	
		RM_FAILURE	= 0x2
	} tPkgRM;

	typedef enum {
		pkgType_RPM,
		pkgType_Tarball,
		pkgType_Zip,
		pkgType_None
	} tPkgType;

	typedef struct tFileData {
		char *data;
		size_t s;
	};

protected:
	UT_sint32 			getLangNum(const char *szName);
	void 				startElement(const XML_Char* name, const XML_Char **atts);
	void 				endElement(const XML_Char *name);
	void 				charData(const XML_Char*, int);
	UT_sint32			tryToDownloadHash(XAP_Frame *pFrame, const char *szFName, XAP_HashDownloader::tFileData *fileData);
	virtual UT_sint32 	downloadFile(XAP_Frame *pFrame, const char *szURL, const char *szDescription, XAP_HashDownloader::tFileData *d, UT_uint32 show_progress) = 0;
	UT_sint32			getPref(XAP_Frame *pFrame);
	UT_sint32			setPref(XAP_Frame *pFrame, UT_sint32 newVal);
	UT_sint32			getComparableBuildDate(void);
	void 				initData(void);
	UT_uint32			dlg_askDownload(XAP_Frame *pFrame, const char *szLang);
	UT_uint32			dlg_askFirstTryFailed(XAP_Frame *pFrame);
	
	virtual void		showNoteDlg(XAP_Frame *pFrame, XAP_String_Id errMsg);
	virtual void 		showNoteDlg(XAP_Frame *pFrame, const char *szMsg);
	
	virtual void		showErrorMsg(XAP_Frame *pFrame, const char *errMsg, bool showErrno=false) const = 0;

	virtual UT_sint32	downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload) = 0;
	virtual tPkgType	wantedPackageType(XAP_Frame *pFrame) = 0;
	virtual UT_sint32	installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, tPkgType pkgType, UT_sint32 rm) = 0;
	
	time_t		dictionaryListMaxAge;
	tFileData 	fileData;
	UT_XML 		xmlParser;
	UT_sint32 	xmlParseOk;
	UT_sint32 	doUse;

private:
	UT_sint32 	xmlParseDepth;
	char 		version[MAX_NUM_LANGUAGES][16];
	char 		release[MAX_NUM_LANGUAGES][16];
	char 		mrd[MAX_NUM_LANGUAGES][10];
	char 		listVersion[16];
	char 		host[256];
	char 		host2[256];
};


#endif
