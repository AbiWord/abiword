/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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


/* WARNING!
 * Do not enable this unless you know what you do!
 * It works, but don't come screaming if Abiword destroyed your harddrive while
 * you where running it as root
 */
/*#define LET_ROOT_INSTALL_SYSTEMWIDE*/

/*
 * If you've enabled the above option and are running a rpm system, you can
 * enable this. If current user is root it will try to install an rpm instead
 * of a tar.gz
 */
/*#define IS_RUNNING_ON_RPM_SYSTEM*/


#define MAX_NUM_LANGUAGES 100


typedef struct fileData_t {
	char *data;
	size_t s;
};


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

protected:
	UT_sint32 	getLangNum(const char *szName);
	void 		startElement(const XML_Char* name, const XML_Char **atts);
	void 		endElement(const XML_Char *name);
	void 		charData(const XML_Char*, int);
	UT_sint32 	downloadFile(XAP_Frame *pFrame, const char *szFName, fileData_t *d, UT_uint32 show_progress);
	UT_sint32	getPref(XAP_Frame *pFrame);
	UT_sint32	setPref(XAP_Frame *pFrame, UT_sint32 newVal);
	UT_sint32	getComparableBuildDate(void);
	void 		initData(void);
	UT_sint32	mySystem(const char *szCommand);
	UT_uint32	dlg_askDownload(XAP_Frame *pFrame, const char *szLang);
	UT_uint32	dlg_askFirstTryFailed(XAP_Frame *pFrame);
	
	virtual	void	showProgressStart(XAP_Frame *pFrame, CURL *ch) = 0;
	virtual	void	showProgressStop(XAP_Frame *pFrame, CURL *ch) = 0;

	virtual UT_sint32	downloadDictionaryList(XAP_Frame *pFrame, const char *endianess, UT_uint32 forceDownload) = 0;
	virtual tPkgType	wantedPackageType(XAP_Frame *pFrame) = 0;
	virtual UT_sint32	installPackage(XAP_Frame *pFrame, const char *szFName, const char *szLName, tPkgType pkgType, UT_sint32 rm) = 0;
	
	time_t		dictionaryListMaxAge;
	fileData_t 	fileData;
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
};


#endif
