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

#include <stdlib.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "ap_Dialog_Field.h"
#include "ap_QNXDialog_Field.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

static int s_ok_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Field * dlg = (AP_QNXDialog_Field *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Field * dlg = (AP_QNXDialog_Field *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_types_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Field * dlg = (AP_QNXDialog_Field *)data;
	UT_ASSERT(widget && dlg);
	if(info->reason_subtype == Pt_LIST_SELECTION_FINAL) {
		PtListCallback_t *lcb = (PtListCallback_t *)info->cbdata;
		dlg->types_changed(lcb->item_pos -1);
	}
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info) 
{
	AP_QNXDialog_Field * dlg = (AP_QNXDialog_Field *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}


/*****************************************************************/


XAP_Dialog * AP_QNXDialog_Field::static_constructor(XAP_DialogFactory * pFactory,
													XAP_Dialog_Id id)
{
	AP_QNXDialog_Field * p = new AP_QNXDialog_Field(pFactory,id);
	return p;
}

AP_QNXDialog_Field::AP_QNXDialog_Field(XAP_DialogFactory * pDlgFactory,
									   XAP_Dialog_Id id)
	: AP_Dialog_Field(pDlgFactory,id)
{
}

AP_QNXDialog_Field::~AP_QNXDialog_Field(void)
{
}

void AP_QNXDialog_Field::runModal(XAP_Frame * pFrame)
{
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);	

	UT_ASSERT(pFrame);
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	connectFocus(mainWindow,pFrame);

	// Populate the window's data items
	_populateCatogries();

    UT_QNXCenterWindow(parentWindow, mainWindow);
    UT_QNXBlockWidget(parentWindow, 1);
    PtRealizeWidget(mainWindow);

    int count;
    count = PtModalStart();
    done = 0;
    while(!done) {
            PtProcessEvent();
    }
    PtModalEnd(MODAL_END_ARG(count));

    UT_QNXBlockWidget(parentWindow, 0);
    PtDestroyWidget(mainWindow);
}


void AP_QNXDialog_Field::event_OK(void)
{
	unsigned short *index;

	UT_ASSERT(m_windowMain && m_listTypes && m_listFields);
	// find item selected in the Types list box, save it to m_iTypeIndex

	//TypeIndex should be set properly already, so don't bother with it
	
	// find item selected in the Field list box, save it to m_iFormatIndex
	index = NULL;
	PtGetResource(m_listFields, Pt_ARG_SELECTION_INDEXES, &index, 0);

	//Nothing selected, act like a cancel
	if (!index || index[0] == 0) {
		printf("Hey ... nothing selected 2 \n");
		m_answer = AP_Dialog_Field::a_CANCEL;
		done++;
		return;
	}

	m_iFormatIndex = index[0] - 1;

	//I protest!  This is totally wacked that I have to loop through 
	//looking at what entry matches.  This should _totally_ by XP code
	//The format is the index of the format in the formate field rather than
	//the index of the format that fits that type.  Very misleading!
	for (int i = 0; fp_FieldFmts[i].m_Tag != NULL; i++) 
	{
		if (fp_FieldFmts[i].m_Type == fp_FieldTypes[m_iTypeIndex].m_Type)
		{
 			if(m_iFormatIndex == 0) {
				m_iFormatIndex = i;
				break;
			}
			m_iFormatIndex--;
		}
	}


	m_answer = AP_Dialog_Field::a_OK;
	done++;
}


void AP_QNXDialog_Field::types_changed(int row)
{
	UT_ASSERT(m_windowMain && m_listTypes);

	// Update m_iTypeIndex with the row number
	m_iTypeIndex = row;

	// Update the fields list with this new Type
	setFieldsList();
}

void AP_QNXDialog_Field::event_Cancel(void)
{
	if(!done++) {
		m_answer = AP_Dialog_Field::a_CANCEL; 
	}
}

void AP_QNXDialog_Field::event_WindowDelete(void)
{
	if(!done++) {
		m_answer = AP_Dialog_Field::a_CANCEL;	
	}
}


void AP_QNXDialog_Field::setTypesList(void)
{
	UT_ASSERT(m_listTypes);

	int i;

	for (i=0; fp_FieldTypes[i].m_Desc != NULL; i++) {
		PtListAddItems(m_listTypes, &fp_FieldTypes[i].m_Desc, 1, 0);
	}

	PtListSelectPos(m_listTypes, 1);
	m_iTypeIndex = 0;
}

void AP_QNXDialog_Field::setFieldsList(void)
{
	UT_ASSERT(m_listFields);
	fp_FieldTypesEnum  FType = fp_FieldTypes[m_iTypeIndex].m_Type;
	int i;

	PtListDeleteAllItems(m_listFields);

	for (i = 0; fp_FieldFmts[i].m_Tag != NULL; i++) 
	{
		if (fp_FieldFmts[i].m_Type == FType)
		{
			PtListAddItems(m_listFields, &fp_FieldFmts[i].m_Desc, 1, 0);
		}
	}

	PtListSelectPos(m_listFields, 1);
}


/*****************************************************************/


PtWidget_t * AP_QNXDialog_Field::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();


	// Start with the main window
	m_windowMain = abiCreatePhabDialog("ap_QNXDialog_Field",pSS,AP_STRING_ID_DLG_Field_FieldTitle);
	SetupContextHelp(m_windowMain,this);
	PtAddHotkeyHandler(m_windowMain,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(m_windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	// Label the Types Box
localizeLabel(abiPhabLocateWidget(m_windowMain,"lblTypes"), pSS, AP_STRING_ID_DLG_Field_Types );

	localizeLabel(abiPhabLocateWidget(m_windowMain,"lblFields"), pSS, AP_STRING_ID_DLG_Field_Fields );

	// Put a scrolled window into the Types box
	m_listTypes = abiPhabLocateWidget(m_windowMain,"listTypes"); 
	PtAddCallback(m_listTypes, Pt_CB_SELECTION, s_types_clicked, this);

	// Put a scrolled window into the Fields box
	m_listFields = abiPhabLocateWidget(m_windowMain,"listFields"); 

	PtWidget_t *buttonCancel = abiPhabLocateWidget(m_windowMain,"btnCancel"); 
	localizeLabel(buttonCancel, pSS, XAP_STRING_ID_DLG_Cancel);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	PtWidget_t *buttonOK = abiPhabLocateWidget(m_windowMain,"btnOK"); 
	localizeLabel(buttonOK, pSS, XAP_STRING_ID_DLG_OK);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	return m_windowMain;
}

void AP_QNXDialog_Field::_populateCatogries(void)
{
	// Fill in the two lists
	setTypesList();
	setFieldsList();
}
	

