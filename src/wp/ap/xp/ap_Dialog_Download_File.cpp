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
	}
	return 0;
}

AP_Dialog_Download_File_Thread::AP_Dialog_Download_File_Thread(const char *szFName, int dataPipe[2], AP_Dialog_Download_File::tProgressData *pd)
	: UT_Thread(UT_Thread::PRI_NORMAL)
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
	
	if (!ret) {
		/* Feed the main-thread with the filedata through this pipe */
		write(m_dataPipe[1], m_data.data, m_data.s);
	}

	UT_DEBUGMSG(("Download thread is now exiting\n"));
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
	
	m_pG = NULL;
	s_iPBFixedHeight = 20;
	_setHeight(s_iPBFixedHeight);
	// Minimum wanted width
	_setWidth(250);
	
	_reflowPBRect();
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
	if (getUserAnswer() == a_CANCEL)
		return;		// user pressed cancel right under our nose, exit
	
	if (getDLDone()) {
		/* The downloadthread is done, so it's time to destroy the dialog and clean up */
		
		if (_getDialogRemoved())
			return;		// We have already done this, exit
		
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

	/* Filedata pipe */
	pipe(dataPipe);

	th = new AP_Dialog_Download_File_Thread(getURL(), dataPipe, m_pd);

	/* Timer that checks if download is done and updates the progressmeeter */
	tim = UT_Timer::static_constructor(AP_Dialog_Download_File_timerCallback, this);
	tim->set(100);

	/* Start thread - in other words, start download */
	th->start();
	
	/*
	 * Display dialog
	 * This will run untill the user presses cancel or the download is done
	 */
	_runModal(pFrame);
	
	tim->stop();
	DELETEP(tim);

	/* If everything went OK, get filedata from other thread */
	if (getUserAnswer() != a_CANCEL && getDLDone() && !getDLResult()) {
		m_data.s = getFileSize();
		if (!(m_data.data = (char *)malloc(m_data.s))) {
			_setDLResult(-1);
			return;
		}
		read(dataPipe[0], m_data.data, m_data.s);
	}
}

void 
AP_Dialog_Download_File::_reflowPBRect(void)
{
	memset(&m_rect3d,0,sizeof(m_rect3d));
	m_rect3d.left	= 3;
	m_rect3d.width	= _getWidth() - 2*m_rect3d.left;
	m_rect3d.top	= 3;
	m_rect3d.height	= _getHeight() - 2*m_rect3d.top;
}

void 
AP_Dialog_Download_File::_drawPB3D(void)
{
	m_pG->fillRect(GR_Graphics::CLR3D_Background,
				 m_rect3d.left,m_rect3d.top,
				 m_rect3d.width,m_rect3d.height);

	UT_uint32 l = m_rect3d.left -1;
	UT_uint32 r = l + m_rect3d.width +2;
	UT_uint32 t = m_rect3d.top -1;
	UT_uint32 b = t + m_rect3d.height +2;
	
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	m_pG->drawLine(l,t, l,b);
	m_pG->drawLine(l,t, r,t);
	
	m_pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	m_pG->drawLine(l+1,b, r,b);
	m_pG->drawLine(r,b, r,t);
}

void 
AP_Dialog_Download_File::_drawPB(void)
{
	UT_RGBColor clr(255,255,0);			//Yellow
	UT_RGBColor anticlr(120,120,120);	//Dark grey
	UT_Rect newrect;
	UT_Rect greyrect;

	m_pG->setClipRect(&m_rect3d);

	char buffer[AP_MAX_MESSAGE_FIELD];

	buffer[0] = '\0';
	newrect = 
	greyrect = m_rect3d;

	//TODO: Get rid of the double here ...
	double percent = (double)getProgress() / (double)getFileSize(); 
	newrect.width = (UT_sint32)((double)newrect.width * percent); 

	greyrect.left += newrect.width;
	greyrect.width -= newrect.width;

	m_pG->fillRect(clr, newrect);
	m_pG->fillRect(anticlr, greyrect);

	sprintf(buffer, "%.1f%%", 100 * percent); 

	UT_sint32 len = strlen(buffer);
	UT_UCSChar bufUCS[AP_MAX_MESSAGE_FIELD];
	UT_UCS4_strcpy_char(bufUCS, buffer);

	UT_uint32 iFontHeight = m_pG->getFontHeight();
	UT_uint32 iStringWidth = m_pG->measureString(bufUCS, 0, len, NULL);

	UT_uint32 x = m_rect3d.left + 3;
	UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

	x = m_rect3d.left + ((m_rect3d.width - iStringWidth) / 2);

	m_pG->setColor3D(GR_Graphics::CLR3D_Foreground);

	m_pG->setClipRect(&m_rect3d);
	m_pG->drawChars(bufUCS, 0, len, x, y);
	m_pG->setClipRect(NULL);
}

void 
AP_Dialog_Download_File::_updateProgress(XAP_Frame *pFrame)
{
	if (!m_pG)
		return;
	
	// draw the background
	m_pG->fillRect(GR_Graphics::CLR3D_Background, 0, 0, m_iWidth, m_iHeight);
	_drawPB3D();
	
	if (getProgress() == -1)
		return;
	
	// draw the foreground
	_drawPB();
}


/*
 * These are used for the non-threaded version, to show the progress on the statusbar
 */
void
AP_Dialog_Download_File::_showProgressStart(XAP_Frame *pFrame)
{
	AP_FrameData *pFrameData;
	pFrameData = static_cast<AP_FrameData *> (pFrame->getFrameData());
	UT_ASSERT(pFrameData);

	// turn the view status bit on
	pFrameData->m_pStatusBar->setStatusProgressType(0, 100, PROGRESS_STARTBAR);
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
	pFrameData->m_pStatusBar->setStatusProgressType(0, 100, PROGRESS_STOPBAR);
	pFrameData->m_bShowStatusBar = didShowStatusBar;
	pFrame->toggleStatusBar(didShowStatusBar);
}


UT_sint32 
AP_Dialog_Download_File::getFileData(char *buf, UT_uint32 offset, UT_uint32 len)
{
	memcpy(buf, &m_data.data[offset], len);
	return(0);
}
