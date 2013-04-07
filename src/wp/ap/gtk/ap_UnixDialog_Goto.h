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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
	void onXMLIDDblClicked (void);
	void onAnnoDblClicked (void);
	void onJumpClicked 		  (void);
	void onPrevClicked 		  (void);
	void onNextClicked 		  (void);

	void updateCache		  (AP_JumpTarget target);
	void updateDocCount 	  (void);
	void updatePosition		  (void);

	const GtkWidget *getWindow (void) { return m_wDialog; }

protected:

	void _constructWindow 	  (XAP_Frame *pFrame);
	void _updateWindow		  (void);

private:

	enum {
		COLUMN_NAME = 0,  /* currently only one column
		COLUMN_PAGE,
		COLUMN_NUMBER,  */
		NUM_COLUMNS
	};
	enum {
        COLUMN_ANNO_ID = 0,
        COLUMN_ANNO_TITLE,
        COLUMN_ANNO_AUTHOR,
		NUM_ANNO_COLUMNS
    };


	void  _selectPrevBookmark 		 (void);
	void  _selectNextBookmark 		 (void);
    std::string _getSelectedBookmarkLabel();
	std::string _getSelectedXMLIDLabel();
	std::string _getSelectedAnnotationLabel();

	GtkWidget *m_wDialog;
	GtkWidget *m_nbNotebook;
	GtkWidget *m_lbPage;
	GtkWidget *m_lbLine;
	GtkWidget *m_lbBookmarks;
	GtkWidget *m_lbXMLids;
	GtkWidget *m_lbAnnotations;
	GtkWidget *m_sbPage;
	GtkWidget *m_sbLine;
	GtkWidget *m_lvBookmarks;
	GtkWidget *m_btJump;
	GtkWidget *m_btPrev;
	GtkWidget *m_btNext;
    GtkWidget *m_lvXMLIDs;
    GtkWidget *m_lvAnno;
	GtkWidget *m_btClose;

	guint m_iPageConnect;
	guint m_iLineConnect;
	AP_JumpTarget m_JumpTarget;
	FV_DocCount   m_DocCount;

    void setupXMLIDList( GtkWidget* w );
    void updateXMLIDList( GtkWidget* w );
    void setupAnnotationList( GtkWidget* w );
    void updateAnnotationList( GtkWidget* w );
};

#endif /* AP_UNIXDIALOG_GOTO_H */



