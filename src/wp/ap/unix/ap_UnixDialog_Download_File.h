/* Abiword
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

#ifndef AP_UNIXDIALOG_DOWNLOAD_FILE_H
#define AP_UNIXDIALOG_DOWNLOAD_FILE_H

#include "ap_Dialog_Download_File.h"
#include "xap_UnixFrame.h"
#include "gr_UnixGraphics.h"

/*****************************************************************/

class ABI_EXPORT AP_UnixDialog_Download_File : public AP_Dialog_Download_File
{
public:
	AP_UnixDialog_Download_File(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Download_File(void);

	static XAP_Dialog	*static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	virtual void		_runModal(XAP_Frame * pFrame);
	virtual void		_abortDialog(void);	/* Called when download is finished and dialog should be remobed */

 protected:
	typedef enum
	{
		BUTTON_CANCEL
	} ResponseId ;
		
	GtkWidget			*_constructWindow(void);

	// for easy Gnome overriding
	virtual GtkWidget 	*_constructProgressBar(void);

	void				_populateWindowData(void);
	
	virtual void		_updateProgress(XAP_Frame *pFrame);
	
	GtkWidget			*m_windowMain;
	GtkWidget			*m_progressBar;
	
	GR_UnixGraphics		*m_gc;
	XAP_Frame 		*m_pFrame;
};

#endif /* AP_UNIXDIALOG_DOWNLOAD_FILE_H */
