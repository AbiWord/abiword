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

#ifndef AP_DIALOG_LISTS_H
#define AP_DIALOG_LISTS_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "fl_BlockLayout.h"
#include "fl_AutoNum.h"
#include "pt_Types.h"

class FV_View;
class fl_AutoNum;
class fl_BlockLayout;
		
class AP_Dialog_Lists : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Lists(void);

	// these are kinda screwy now, but we never return anything but on
	// "cancel" or "close"
	typedef enum {
		a_CLOSE
	} tAnswer;

        AP_Dialog_Lists::tAnswer	 	getAnswer(void) const;
        void                                    ConstructWindowName(void);
        void                                    StartList(void);
        void                                    StopList(void);
        void                                    Apply(void);
        void                                    PopulateDialogData(void);
        UT_Bool                                 isLastOnLevel(void);
	XML_Char *                              getListStyleString(void);
        fl_AutoNum *                            getAutoNum(void);
	fl_BlockLayout *                        getBlock(void);

  	UT_Bool						setView(FV_View * view);
  	FV_View * 					getView(void);
	void                                            setActiveFrame(XAP_Frame *pFrame);

 protected:
	
	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.
  	FV_View * 					m_pView;
	// is this used in a modeless dialog like this?
	tAnswer						m_answer;
	char                                            m_WindowName[100];
	UT_Bool                                         m_isListAtPoint;
	UT_Bool                                         m_previousListExistsAtPoint;
        char                                            m_curListType[100];
	char                                            m_newListType[100];
        char                                            m_curListLabel[100];
	char                                            m_newListLabel[100];
	UT_uint32                                       m_curStartValue;
	UT_uint32                                       m_newStartValue;
        UT_uint32                                       m_curListLevel;
        UT_uint32                                       m_newListLevel;
        UT_uint32                                       m_iListType;
 
	UT_Bool                                         m_bStartList;
	UT_Bool                                         m_bStopList;
        UT_Bool                                         m_bChangeStartValue;
};

#endif /* AP_DIALOG_LISTS_H */

