/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef AP_QNXDIALOG_WORDCOUNT_H
#define AP_QNXDIALOG_WORDCOUNT_H

#include "ap_Dialog_WordCount.h"
#include "ut_timer.h"

class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_WordCount: public AP_Dialog_WordCount
{
public:
	AP_QNXDialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_WordCount(void);

	//DEPRECATED
	virtual void			runModal(XAP_Frame * pFrame);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			notifyActiveFrame(XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static void				autoupdateWC(UT_Timer * pTimer);

	virtual void			setUpdateCounter(void);

	virtual void			event_OK(void);
	virtual void			event_Update(void);
	virtual void			event_Checkbox(void);
	virtual void			event_Spin(void);

	virtual void			event_WindowDelete(void);
	
protected:

	// private construction functions
	virtual PtWidget_t * _constructWindow(void);
	void 				 _updateWindowData(void);       

	//DEPRECATED
	void				_populateWindowData(void);

	// pointers to widgets we need to query/set
	PtWidget_t * m_windowMain;
	//PtWidget_t * m_buttonClose;
	//PtWidget_t * m_buttonUpdate;
	//PtWidget_t * m_wContent;
	//PtWidget_t * m_pTableframe;
	PtWidget_t * m_pAutospin;
	PtWidget_t * m_pAutocheck;
	//PtWidget_t * m_pAutospinlabel;

	// Labels for the Word Count data
	PtWidget_t * m_labelWCount;
	PtWidget_t * m_labelPCount;
	PtWidget_t * m_labelCCount;
	PtWidget_t * m_labelCNCount;
	PtWidget_t * m_labelLCount;	
	PtWidget_t * m_labelPgCount;	
	
	UT_Timer   * m_pAutoUpdateWC;
	UT_Bool		 m_bAutoWC;
	int			 m_Update_rate;

	// Handshake variables
	UT_Bool m_bDestroy_says_stopupdating;
	UT_Bool m_bAutoUpdate_happening_now;

	int 		done;
};

#endif /* AP_QNXDIALOG_WORDCOUNT_H */
