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


#ifdef WIN32
#include <io.h>		// _pipe
#include <fcntl.h>	// _O_BINARY
#define pipe(x) _pipe(x, 256, _O_BINARY)
#else
#include <unistd.h>	// pipe
#endif

#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"

#include "ap_Strings.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"
#include "ap_Dialog_Download_File.h"

#include "ut_worker.h"
#include "ut_string.h"
#include "ut_path.h"
#include "ut_timer.h"


size_t
AP_Dialog_Download_File_Thread_writeCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	AP_Dialog_Download_File::tFileData *mem = (AP_Dialog_Download_File::tFileData *)data;

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

int 
AP_Dialog_Download_File_Thread_progressCallback(AP_Dialog_Download_File_Thread *dlg, double total, double dl, double ultotal, double ulnow)
{
	return( dlg->event_Progress((UT_sint32)total, (UT_sint32)dl) );
}

UT_sint32
AP_Dialog_Download_File_Thread::event_Progress(UT_sint32 total, UT_sint32 progress)
{
	if (_getUserAnswer() == AP_Dialog_Download_File::a_CANCEL) {
		/*
		 * User pressed the cancel-button, abort download
		 * => abort the curl-download through returning a negative value
		 */
		return(-1);	
	}
	
	/* 
	 * During the first seconds of download this function may be called with total==0
	 * This would turn on "constant-progress" on the progressbar, we don't want that
	 */
	if (total == 0) {
		_setProgress(-1);
		_setFileSize(-1);
	} else {
		_setProgress(progress);
		_setFileSize(total);

#ifndef HAVE_THREADS
		XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
		UT_ASSERT(pFrame);
		AP_FrameData *pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
		UT_ASSERT(pFrameData);

		pFrameData->m_pStatusBar->setStatusProgressType(0, total, PROGRESS_START | PROGRESS_SHOW_PERCENT);
		pFrameData->m_pStatusBar->setStatusProgressValue(progress);
#endif
	}
	return 0;
}

AP_Dialog_Download_File_Thread::AP_Dialog_Download_File_Thread(const char *szFName, int dataPipe[2], AP_Dialog_Download_File::tProgressData *pd)
#ifdef HAVE_THREADS
	: UT_Thread(UT_Thread::PRI_NORMAL)
#endif
{
	m_szFName = UT_strdup(szFName);
	m_dataPipe[0] = dataPipe[0];
	m_dataPipe[1] = dataPipe[1];
	m_pd = pd;
	m_showProgress = 1;
}

AP_Dialog_Download_File_Thread::~AP_Dialog_Download_File_Thread()
{
	FREEP(m_szFName);
	FREEP(m_data.data);
}

void 
AP_Dialog_Download_File_Thread::run()
{
	CURL *ch;
	UT_sint32 ret = 0;
	
	m_data.data = NULL;
	m_data.s = 0;
	
	/* init the curl session */
	ch = curl_easy_init();
	curl_easy_setopt(ch, CURLOPT_URL, m_szFName);
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, AP_Dialog_Download_File_Thread_writeCallback);
	curl_easy_setopt(ch, CURLOPT_FAILONERROR, 1);
	/* Data to send to AP_Dialog_Download_File_Thread_writeCallback() */
	curl_easy_setopt(ch, CURLOPT_FILE, (void *)&m_data);

	if (m_showProgress) {
		curl_easy_setopt(ch, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, AP_Dialog_Download_File_Thread_progressCallback);
		curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, this);
	} else
		curl_easy_setopt(ch, CURLOPT_NOPROGRESS, TRUE);

	UT_DEBUGMSG(("Starting download of %s\n", m_szFName));
	/* get it! */
	if ((ret = curl_easy_perform(ch))) {
		UT_DEBUGMSG(("Download of %s failed! (ret=%d)\n", m_szFName, ret));
	} else {
		UT_DEBUGMSG(("Done downloading %s\n", m_szFName));
	}

	if (m_showProgress)
		curl_easy_setopt(ch, CURLOPT_NOPROGRESS, TRUE);

	//TODO: figure out why I crash on Windows
//#ifndef _WIN32
	curl_easy_cleanup(ch);
//#endif
	
	_setDLResult(ret);
	_setDLDone(1);
	
#ifdef HAVE_THREADS	
	if (!ret) {
		/* Feed the main-thread with the filedata through this pipe */
		write(m_dataPipe[1], m_data.data, m_data.s);
	}

	UT_DEBUGMSG(("Download thread is now exiting\n"));
#endif
}


AP_Dialog_Download_File::AP_Dialog_Download_File(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{
	const XAP_StringSet *pSS = XAP_App::getApp()->getStringSet();

	m_pd = (tProgressData *)malloc(sizeof(tProgressData));
	m_pd->totSize = -1;
	m_pd->dlProgress = -1;
	m_pd->dlDone = 0;
	m_pd->dlResult = 0;
	m_pd->userAnswer = a_NONE;
	m_dialogRemoved = 0;
	m_szTitle = pSS->getValue(AP_STRING_ID_DLG_DlFile_Title);
}

AP_Dialog_Download_File::~AP_Dialog_Download_File()
{
	FREEP(m_pd);
	FREEP(m_data.data);
}


static void 
AP_Dialog_Download_File_timerCallback(UT_Worker* pWorker)
{
	((AP_Dialog_Download_File*)(pWorker->getInstanceData()))->event_Timer();
}

void
AP_Dialog_Download_File::event_Timer(void)
{
	if (getDLDone()) {
		/* The downloadthread is done, so it's time to destroy the dialog and clean up */
		
		if (_getDialogRemoved() || getUserAnswer() == a_CANCEL)
			return;		// 1) We have already done this, exit
						// or 2) user pressed cancel right under our nose, exit
		
		_setDialogRemoved(1);
		_abortDialog();
		return;
	}

	if (getProgress() != -1) {
		XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
		UT_ASSERT(pFrame);
		
		/* Update the graphical progressmeeter */
		_updateProgress(pFrame);
	}
}

void
AP_Dialog_Download_File::runModal(XAP_Frame * pFrame)
{
	int dataPipe[2];
	UT_Timer *tim;
	AP_Dialog_Download_File_Thread *th;

#ifdef HAVE_THREADS	
	/* Timer that checks if download is done and updates the progressmeeter */
	tim = UT_Timer::static_constructor(AP_Dialog_Download_File_timerCallback, this);
	tim->set(100);
#endif	
	
	/* Filedata pipe */
	pipe(dataPipe);
	
	/* Init graphical progressmeeter */
	if (getShowProgress())
		_showProgressStart(pFrame);
	
	th = new AP_Dialog_Download_File_Thread(getURL(), dataPipe, m_pd);
	
#ifdef HAVE_THREADS	
	/* Start thread - in other words, start download */
	th->start();
	
	/*
	 * Display dialog
	 * This will run untill the user presses cancel or the download is done
	 */
	_runModal(pFrame);
	
	tim->stop();
	DELETEP(tim);
#else
	/* Runs download method directly, without creating a new thread */
	th->run();
#endif	
	
	if (getShowProgress())
		_showProgressStop(pFrame);

#ifdef HAVE_THREADS	
	/* If everything went OK, get filedata from other thread */
	if (getUserAnswer() != a_CANCEL && getDLDone() && !getDLResult()) {
		m_data.s = getFileSize();
		m_data.data = (char *)malloc(m_data.s);
		read(dataPipe[0], m_data.data, m_data.s);
	}
#else
	/* If everything went OK, get filedata from other thread */
	if (getDLDone() && !getDLResult()) {
		m_data.s = getFileSize();
		m_data.data = th->_getFileData();
	}
#endif
}


void 
AP_Dialog_Download_File::_updateProgress(XAP_Frame *pFrame)
{
	if ( getProgress() == -1)
		return;

	AP_FrameData *pFrameData;
	pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	pFrameData->m_pStatusBar->setStatusProgressType(0, getFileSize(), PROGRESS_START | PROGRESS_SHOW_PERCENT);
	pFrameData->m_pStatusBar->setStatusProgressValue(getProgress());
}

void
AP_Dialog_Download_File::_showProgressStart(XAP_Frame *pFrame)
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
}

void
AP_Dialog_Download_File::_showProgressStop(XAP_Frame *pFrame)
{
	AP_FrameData *pFrameData;
	pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	// turn the view status bit off
	pFrameData->m_pStatusBar->setStatusProgressType(0, 100, PROGRESS_STOP);
	pFrameData->m_bShowStatusBar = didShowStatusBar;
	pFrame->toggleStatusBar(didShowStatusBar);
}

UT_sint32 
AP_Dialog_Download_File::getFileData(char *buf, UT_uint32 offset, UT_uint32 len)
{
	memcpy(buf, &m_data.data[offset], len);
	return(0);
}
