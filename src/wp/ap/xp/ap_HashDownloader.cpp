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

#include <sys/types.h>
// this XP file -- you cannot include unix-only stuff !!!
//#include <unistd.h>
//#include <sys/wait.h>
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

#include "xap_Frame.h"
#include "xap_Strings.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"

#include "ispell_checker.h"
#include "ap_HashDownloader.h"


extern char **environ;


extern "C" {
int 
dlProgressCallback(AP_StatusBar *statusBar, double total, double dl, double ultotal, double ulnow)
{
	/* 
	 * During the first seconds of download this function may be called with total==0
	 * This would turn on "constant-progress" on the progressbar, we don't want that
	 */
	if ( (int)(total) != 0) {
		statusBar->setStatusProgressType(0, (int)(total), PROGRESS_START | PROGRESS_SHOW_PERCENT);
		statusBar->setStatusProgressValue((int)(dl));
	}
	
	return 0;
}
}


AP_HashDownloader::AP_HashDownloader()
{
}

AP_HashDownloader::~AP_HashDownloader()
{
}


void
AP_HashDownloader::showProgressStart(XAP_Frame *pFrame, CURL *ch)
{
	AP_FrameData *pFrameData;
	pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	// turn the view status bit on
	pFrameData->m_pStatusBar->setStatusProgressType(0, 100, PROGRESS_START);
	pFrameData->m_pStatusBar->setStatusProgressValue(0);

	didShowStatusBar = pFrame->isStatusBarShown();
	pFrameData->m_bShowStatusBar = true;
	pFrame->toggleStatusBar(true);

	curl_easy_setopt(ch, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, dlProgressCallback);
	curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, pFrameData->m_pStatusBar);
}

void
AP_HashDownloader::showProgressStop(XAP_Frame *pFrame, CURL *ch)
{
	AP_FrameData *pFrameData;
	pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	// turn the view status bit off
	pFrameData->m_pStatusBar->setStatusProgressType(0, 100, PROGRESS_STOP);
	pFrameData->m_bShowStatusBar = didShowStatusBar;
	pFrame->toggleStatusBar(didShowStatusBar);
}
