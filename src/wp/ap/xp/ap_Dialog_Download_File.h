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

#ifndef AP_DIALOG_DOWNLOAD_FILE_H
#define AP_DIALOG_DOWNLOAD_FILE_H


#include <curl/types.h>
#include "xap_Dialog.h"
#include "xap_Frame.h"
#include "xav_View.h"

#ifdef HAVE_THREADS
#include "ut_thread.h"
#endif

class AP_Dialog_Download_File_Thread;

class ABI_EXPORT AP_Dialog_Download_File : public XAP_Dialog_NonPersistent
{
public:
	typedef enum {a_NONE, a_CANCEL}	tAnswer;

	typedef struct tFileData {
		char *data;
		size_t s;
	};
	
	typedef struct tProgressData {
		UT_sint32	totSize;
		UT_sint32	dlProgress;
		UT_uint32	dlDone;
		UT_sint32	dlResult;
		tAnswer		userAnswer;
	};

	AP_Dialog_Download_File(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	~AP_Dialog_Download_File();
	
	/* This is the one that starts it all
	 * -starts thread and stuff
	 * -calls _runModal()
	 * -waits for download to finish or user to press cancel
	 * -cleans up
	 */
	virtual void		runModal(XAP_Frame * pFrame);

	void				setURL(const char *szURL) { m_szURL = szURL; }
	void				setDescription(const char *szDesc) { m_szDesc = szDesc; }
	virtual void		setShowProgress(UT_uint32 flag) { m_showProgress = flag; }

	const char			*getURL() { return(m_szURL); }
	const char			*getDescription() { return(m_szDesc); }
	UT_uint32			getShowProgress() { return(m_showProgress); }
	
	UT_sint32			getFileData(char *buf, UT_uint32 start, UT_uint32 len);
	UT_sint32			getFileSize() { return(m_pd->totSize); }
	UT_sint32			getProgress() { return(m_pd->dlProgress); }

	tAnswer				getUserAnswer(void) { return(m_pd->userAnswer); }

	/* Flag - has the download-thread completed */
	UT_sint32			getDLDone(void) { return(m_pd->dlDone); }

	/* Resultcode from the download-thread - 0==OK*/
	UT_sint32			getDLResult(void) { return(m_pd->dlResult); }

	void				event_Timer(void);

protected:
	virtual void		_updateProgress(XAP_Frame *pFrame);
	virtual void		_showProgressStart(XAP_Frame *pFrame);
	virtual void		_showProgressStop(XAP_Frame *pFrame);

	/* If user aborts transfer */
	void 				_setUserAnswer(tAnswer ans) { m_pd->userAnswer = ans; }

	void				_setDialogRemoved(UT_uint32 rm) { m_dialogRemoved = rm; }
	UT_uint32			_getDialogRemoved(void) { return(m_dialogRemoved); }

	void				_setProgressData(tProgressData *pd) { m_pd = pd; }

	virtual void		_runModal(XAP_Frame * pFrame) = 0;	/* Redefined in platform specific code */
	virtual void		_abortDialog(void) = 0;		/* Called when download is finished and dialog should be remobed */

	tFileData		m_data;
	UT_sint32 		didShowStatusBar;
	tProgressData	*m_pd;

private:
	const char 		*m_szURL;
	const char		*m_szDesc;
	UT_uint32		m_showProgress;
	UT_uint32		m_dialogRemoved;
};

class ABI_EXPORT AP_Dialog_Download_File_Thread
#ifdef HAVE_THREADS
 : public UT_Thread
#endif
{
 public:
	AP_Dialog_Download_File_Thread(const char *szFName, int dataPipe[2], AP_Dialog_Download_File::tProgressData *pd);
	~AP_Dialog_Download_File_Thread();

	UT_sint32							getFileSize() { return(m_data.s); }
	void								setShowProgress(UT_uint32 flag) { m_showProgress = flag; }

	char								*_getFileData() { return(m_data.data); };
	void								run();
	
	UT_sint32							event_Progress(UT_sint32 total, UT_sint32 progress);
	
 protected:
	void								_setProgress(UT_sint32 progress) { m_pd->dlProgress = progress; }
	void								_setFileSize(UT_sint32 size) { m_pd->totSize = size; }
	AP_Dialog_Download_File::tAnswer	_getUserAnswer() { return(m_pd->userAnswer); }
	void								_setDLDone(UT_sint32 flag) { m_pd->dlDone = flag; }
	void								_setDLResult(UT_sint32 res) { m_pd->dlResult = res; }

 private:
	const char 								*m_szFName;
	AP_Dialog_Download_File::tFileData		m_data;
	int 									m_dataPipe[2];
	UT_uint32								m_showProgress;
	AP_Dialog_Download_File::tProgressData	*m_pd;
};


#endif
