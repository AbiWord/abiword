/* AbiSource Application Framework
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

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include <stdio.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <expat.h>

#include "ispell.h"
#include "ut_iconv.h"

#include "sp_spell.h"

#include "xap_App.h"
#include "ut_string_class.h"

#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_endian.h"

#include "xap_Frame.h"
#include "xap_Strings.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"

#include "ispell_checker.h"
#include "xap_HashDownloader.h"


extern char **environ;


extern "C" {
size_t
writeCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	fileData_t *mem = (fileData_t *)data;

	if (!mem->data)
		mem->data = (char *)malloc(size * nmemb + 1);
	else
		mem->data = (char *)realloc(mem->data, mem->s + size * nmemb + 1);

	if (mem->data) {
		memcpy(&(mem->data[mem->s]), ptr, size * nmemb);
		mem->s += size * nmemb;
		mem->data[mem->s] = 0;
	} else
		return(0);

	return(size * nmemb);
}
}


XAP_HashDownloader::XAP_HashDownloader()
{
	dictionaryListMaxAge = 60*60*1;	/* One hour */
}

XAP_HashDownloader::~XAP_HashDownloader()
{
}

UT_sint32
XAP_HashDownloader::getLangNum(const char *szName)
{
	UT_uint32 i;
	
	for (i = 0; i < (sizeof (m_mapping) / sizeof (m_mapping[0])); i++) {
		if (!strcmp(m_mapping[i].lang, szName))
			break;
	}
	
	if (i == (sizeof (m_mapping) / sizeof (m_mapping[0])))
		return(-1);

	return(i);
}


void
XAP_HashDownloader::startElement(const XML_Char* name, const XML_Char **atts)
{
	UT_sint32 i, l = 0;
	char *endptr;
	
	if (xmlParseOk == -1)
		return;
	
	if (!strcasecmp(name, "Dictionarylist") && xmlParseDepth == 0) {
		for (i = 0; atts[i]; i += 2) {
		
			/* Version of dictionary list */
			if (!strcasecmp(atts[i], "version")) {
				strncpy(listVersion, atts[i + 1], 16);
				listVersion[15] = 0;
			}
			
			/* Host to download dictionaries from */
			if (!strcasecmp(atts[i], "host")) {
				strncpy(host, atts[i + 1], 255);
				host[255] = 0;
			}
			
			/* Flag to indicate if we should use this featur.
			   This allows us to disable the feature later on if we
			   would find out it causes trouble */
			if (!strcasecmp(atts[i], "use")) {
				doUse = strtol(atts[i+1], &endptr, 10);
				if (endptr == atts[i+1])
					xmlParseOk = -1;
			}
		}

		UT_DEBUGMSG(("<DictionaryList v=%s h=%s use=%d>\n", listVersion, host, doUse));
	}

	if (!strcasecmp(name, "Dict") && xmlParseDepth == 1) {
		l = -1;
		
		for (i = 0; atts[i]; i += 2)
			/* Package abbrevation */
			if (!strcasecmp(atts[i], "name")) {
				l = getLangNum(atts[i + 1]);
				break;
			}
		
		if (l != -1) {
			for (i = 0; atts[i]; i += 2) {
				/* Version nr of dictionary package */
				if (!strcasecmp(atts[i], "version")) {
					strncpy(version[l], atts[i + 1], 16);
					version[l][15] = 0;
				}
				
				/* Release nr of dictionary package */
				if (!strcasecmp(atts[i], "release")) {
					strncpy(release[l], atts[i + 1], 16);
					release[l][15] = 0;
				}
				
				/* Minimum required date for this dictionary to work
				   Could for example be a namechange of the hash or
				   a hash that was newly added to the Abi source */
				if (!strcasecmp(atts[i], "mrd")) {
					strncpy(mrd[l], atts[i + 1], 10);
					mrd[l][9] = 0;
				}
			}
			
			UT_DEBUGMSG(("	<Dict v=%s r=%s mrd=%s>\n", version[l], release[l], mrd[l]));
		}
		
		
	}
	
	xmlParseDepth++;
}

void
XAP_HashDownloader::endElement(const XML_Char *name)
{
	xmlParseDepth--;
}

void
XAP_HashDownloader::charData(const XML_Char*, int i)
{
}

void
XAP_HashDownloader::initData()
{
	UT_uint32 i;
	
	xmlParseDepth = 0;
	xmlParseOk = 0;

	for (i = 0; i < MAX_NUM_LANGUAGES; i++) {
		version[i][0] = 0;
		release[i][0] = 0;
		mrd[i][0] = 0;
	}

	listVersion[0] = 0;
	host[0] = 0;
}

UT_sint32
XAP_HashDownloader::downloadFile(XAP_Frame *pFrame, const char *szFName, fileData_t *d, UT_uint32 show_progress)
{
	CURL *ch;
	UT_sint32 ret = 0;
	
	if (d->data)
		free(d->data);
	d->data = NULL;
	d->s = 0;
	
	/* init the curl session */
	ch = curl_easy_init();
	curl_easy_setopt(ch, CURLOPT_URL, szFName);
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(ch, CURLOPT_FAILONERROR, 1);
	
	/* Data to send to writeCallBack() */
	curl_easy_setopt(ch, CURLOPT_FILE, (void *)d);

	if (show_progress)
		showProgressStart(pFrame, ch);
	else
		curl_easy_setopt(ch, CURLOPT_NOPROGRESS, TRUE);

	/* get it! */
	ret = curl_easy_perform(ch);

	if (show_progress)
		showProgressStop(pFrame, ch);

	curl_easy_cleanup(ch);

	return ret;
}


UT_sint32
XAP_HashDownloader::getPref(XAP_Frame *pFrame)
{
	char *endptr, *val;
	UT_sint32 i;
	XAP_Prefs *prefs = pFrame->getApp()->getPrefs();
	XAP_PrefsScheme *prefsScheme = prefs->getCurrentScheme(true);
	
	if (!prefsScheme->getValue((const XML_Char *)"SpellUseHashDownloader", (const XML_Char **)&val))
		return(-1);
	
	i = strtol(val, &endptr, 10);
	if (endptr == val)
		return(-1);
	
	return(i);
}


UT_sint32
XAP_HashDownloader::setPref(XAP_Frame *pFrame, UT_sint32 newVal)
{
	char val[10];
	XAP_Prefs *prefs = pFrame->getApp()->getPrefs();
	UT_ASSERT(prefs);
	XAP_PrefsScheme *prefsScheme = prefs->getCurrentScheme(true);
	UT_ASSERT(prefsScheme);

	sprintf(val, "%d", newVal);
	
	if (!prefsScheme->setValue((const XML_Char *)"SpellUseHashDownloader", (const XML_Char *)val)) {
		fprintf(stderr, "Failed to save preference \"SpellUseHashDownloader\"\n");
		return(-1);
	}

	return(0);
}


UT_sint32
XAP_HashDownloader::getComparableBuildDate()
{
	char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char *buildDate = (char *)XAP_App::getApp()->s_szBuild_CompileDate;
	char month[4], day[3], year[5], all[20];
	char *endptr;
	UT_sint32 i, nMonth, nDay, nYear;
	
	memcpy(month, buildDate, 3);
	month[3] = 0;
	
	for (i = 0; i < 12; i++)
		if (!strcmp(month, months[i])) {
			nMonth = i + 1;
			break;
		}
	if (i == 12)
		return(-1);
	
	memcpy(day, &buildDate[4], 2);
	day[2] = 0;
	
	memcpy(year, &buildDate[7], 4);
	year[4] = 0;
	
	sprintf(all, "%s%02d%s", year, nMonth, day);
	
	i = strtol(all, &endptr, 10);
	if (endptr == all)
		return(-1);
	
	return(i);
}

UT_sint32
XAP_HashDownloader::mySystem(const char *szCommand)
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


UT_uint32
XAP_HashDownloader::dlg_askDownload(XAP_Frame *pFrame, const char *szLang)
{
	char msg[512];
	
	sprintf(msg, "I could not find a dictionary for the language %s.\n"
		"Would you like me to try downloading it from the Internet?", szLang);
			
	if (XAP_Dialog_MessageBox::a_NO == pFrame->showMessageBox(
				msg, XAP_Dialog_MessageBox::b_YN, XAP_Dialog_MessageBox::a_YES))
		return(0);

	return(1);
}


UT_uint32
XAP_HashDownloader::dlg_askFirstTryFailed(XAP_Frame *pFrame)
{
	if (XAP_Dialog_MessageBox::a_NO == pFrame->showMessageBox(
				"Download failed!\n"
				"Would you like to try again?\n"
				"If yes, make sure you are connected to the Internet."
				, XAP_Dialog_MessageBox::b_YN, XAP_Dialog_MessageBox::a_YES))
		return(0);

	return(1);
}


/*
 * If the option "SpellUseHashDownloader" is == 1 or is nonexistant:
 * Show a messagebox for the user that suggests letting Abiword download
 * the wanted dictionary and install it. If user answers Yes, do it.
 * return:	0  - no, thankyou
 *		1  - ok, done, all OK
 *		<0 - error
 */
UT_sint32
XAP_HashDownloader::suggestDownload(XAP_Frame *pFrame, const char *szLang)
{
	char buff[512], szFName[256], *endptr;
	UT_sint32 i, ret;
	FILE *fp;
	tPkgType pkgType;

#ifdef UT_LITTLE_ENDIAN
	char endianess[5] = "i386";
#else
#ifdef UT_BIG_ENDIAN
	char endianess[5] = "ppc";
#endif
#endif
	fileData.data = NULL;
	doUse = 1;

#ifdef HAVE_PSPELL
#warning Unless you know what you are doing you should have ABI_OPT_CURLHASH=0 on pspell systems
	/* This only works on ispell-systems. Correct me if I'm wrong, please. */
	return(0);
#endif
	
	/* Feature disabled in prefs */
	if (!getPref(pFrame))
		return(0);
	
	if (!dlg_askDownload(pFrame, szLang))
		return(0);

	if ((i = getLangNum(szLang)) == -1)
		return(-1);

	pkgType = wantedPackageType(pFrame);

	if (downloadDictionaryList(pFrame, endianess, 0))
		return(-1);

	setPref(pFrame, doUse);
	
	if (!doUse) {
		/* Feature was just disabled. Give user a note about it.
		   From now on this function will exit at getPref() since
		   this value is saved as a preference */
		if (doUse == -1)
			pFrame->showMessageBox("I'm sorry. This feature is not available anymore."
					, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		else
			pFrame->showMessageBox("I'm sorry. This feature is only available to newer versions of Abiword.\n"
						"You are recommended to upgrade your version of Abiword."
					, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return(0);
	}
	
	if (!version[i]) {
		pFrame->showMessageBox("I'm sorry. The wanted dictionary is not available for download.\n"
				, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return(-1);
	}
	
	if (mrd[i][0]) {
		ret = strtol(mrd[i], &endptr, 10);
		if (endptr == mrd[i])
			return(-1);
		if (ret > getComparableBuildDate()) {
			pFrame->showMessageBox("I'm sorry. The wanted dictionary is only available to newer versions of Abiword.\n"
						"You are recommended to upgrade your version of Abiword."
					, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
			return(-1);
		}
	}
	
	switch (pkgType) {
		case pkgType_RPM:
			sprintf(szFName, "abispell-%s-%s-%s.%s.rpm", szLang, version[i], release[i], endianess);
			break;

		case pkgType_Tarball:
			sprintf(szFName, "abispell-%s.%s.tar.gz", szLang, endianess);
			break;

		case pkgType_Zip:
			sprintf(szFName, "abispell-%s.%s.zip", szLang, endianess);
			break;

		case pkgType_None:
			return(0);	/* User said: No thankyou */
	}

	sprintf(buff, "%s%s", host, szFName);
	if ((ret = downloadFile(pFrame, buff, &fileData, 1)))
		if (dlg_askFirstTryFailed(pFrame))
			if ((ret = downloadFile(pFrame, buff, &fileData, 1))) {
				pFrame->showMessageBox("Download of the dictionary failed!\n"
						, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
				return(-1);
			}

	sprintf(buff, "/tmp/%s", szFName);
	if (!(fp = fopen(buff, "wb"))) {
		perror("suggest_download(): fopen(buff, \"wb\") failed");
		return(-1);
	}
	fwrite(fileData.data, fileData.s, 1, fp);
	fclose(fp);

	ret = installPackage(pFrame, buff, szLang, pkgType, RM_SUCCESS | RM_FAILURE);
	if (ret) {
		pFrame->showMessageBox("Installation of the dictionary failed!\n"
				, XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
		return(-2);
	}

	return(1);
}


