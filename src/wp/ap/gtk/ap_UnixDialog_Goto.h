/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) Robert Staudinger <robsta@stereolyzer.net>
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

#ifndef AP_UNIXDIALOG_GOTO_H
#define AP_UNIXDIALOG_GOTO_H

#include "ap_Dialog_Goto.h"
#include "fv_View.h"

class XAP_UnixFrame;



class AP_UnixDialog_Goto: public AP_Dialog_Goto
{
public:
	AP_UnixDialog_Goto (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Goto (void);

	static XAP_Dialog *static_constructor (XAP_DialogFactory *, 
										   XAP_Dialog_Id id);

	virtual void runModeless 	   (XAP_Frame *pFrame);
	virtual void notifyActiveFrame (XAP_Frame *pFrame);
	virtual void activate	 	   (void);
	virtual void destroy 		   (void);

	void onPageChanged 		  (void); 
	void onLineChanged 		  (void);
	void onBookmarkDblClicked (void);
	void onJumpClicked 		  (void);
	void onPrevClicked 		  (void);
	void onNextClicked 		  (void);

	void updateCache		  (AP_JumpTarget target);
	void updateDocCount 	  (void);

	const GtkWidget *getWindow (void) { return m_wDialog; }

protected:

	void constuctWindow 	  (XAP_Frame *pFrame);
	void updateWindow		  (void);

private: 

	enum {
		COLUMN_NAME = 0,  /* currently only one column
		COLUMN_PAGE,
		COLUMN_NUMBER,  */
		NUM_COLUMNS
	};

	void  _selectPrevBookmark 		 (void);
	void  _selectNextBookmark 		 (void);
	gchar *_getSelectedBookmarkLabel (void);	
	
	GtkWidget *m_wDialog;
	GtkWidget *m_lbPage;
	GtkWidget *m_lbLine;
	GtkWidget *m_lbBookmarks;
	GtkWidget *m_sbPage;
	GtkWidget *m_sbLine;
	GtkWidget *m_lvBookmarks;
	GtkWidget *m_btJump;
	GtkWidget *m_btPrev;
	GtkWidget *m_btNext;
	GtkWidget *m_btClose;

	AP_JumpTarget m_JumpTarget;
	FV_DocCount   m_DocCount;
};

#endif /* AP_UNIXDIALOG_GOTO_H */



