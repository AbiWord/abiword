/* AbiSource Application Framework
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

#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"

/*****************************************************************/

XAP_DialogFactory::XAP_DialogFactory(XAP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable)
{
	UT_ASSERT(pApp);

	// we are the factory for application-persistent dialogs

	m_pApp = pApp;
	m_pFrame = NULL;
	m_dialogType = XAP_DLGT_APP_PERSISTENT;
	m_nrElementsDlgTable = nrElem;
	m_dlg_table = pDlgTable;
}

XAP_DialogFactory::XAP_DialogFactory(XAP_Frame * pFrame, XAP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable)
{
	UT_ASSERT(pApp);

	// we are a factory for frame-persistent dialogs
	
	m_pApp = pApp;
	m_pFrame = pFrame;
	m_dialogType = XAP_DLGT_FRAME_PERSISTENT;
	m_nrElementsDlgTable = nrElem;
	m_dlg_table = pDlgTable;
}

XAP_DialogFactory::~XAP_DialogFactory(void)
{
	UT_VECTOR_PURGEALL(XAP_Dialog *, m_vecDialogs);
}

bool XAP_DialogFactory::_findDialogInTable(XAP_Dialog_Id id, UT_uint32 * pIndex) const
{
	// search the table and return the index of the entry with this id.

	UT_ASSERT(pIndex);

	for (UT_uint32 k=0; (k < m_nrElementsDlgTable); k++)
	{
		if (m_dlg_table[k].m_id == id)
		{
			*pIndex = k;
			return true;
		}
	}
	UT_DEBUGMSG(("Could not find a match for id %d \n",id));
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return false;
}

/*****************************************************************/

/*!
 * This method just creates a new instance of the dialog without remembering
 * anything about it. It's up the to calling prgram to delete the dialog 
 * when it is finished with it.
\params XAP_Dialog_Id id the identification number of the dialog.
\returns XAP_Dialog * pointer to the new instance of the dialog.
 */
XAP_Dialog * XAP_DialogFactory::justMakeTheDialog(XAP_Dialog_Id id)
{
	UT_uint32 index;
	XAP_Dialog * pDialog = NULL;
	
	if(_findDialogInTable(id,&index))
	{
		pDialog = (XAP_Dialog *)((m_dlg_table[index].m_pfnStaticConstructor)(this,id));
		return pDialog;
	}
	return NULL;
}

XAP_Dialog * XAP_DialogFactory::requestDialog(XAP_Dialog_Id id)
{
	UT_uint32 index;
	XAP_Dialog * pDialog = NULL;
	
	if(_findDialogInTable(id,&index))
	{

		switch (m_dlg_table[index].m_type)
		{
		case XAP_DLGT_NON_PERSISTENT:	// construct a non-persistent dialog and just return it.
			goto CreateItSimple;

		case XAP_DLGT_FRAME_PERSISTENT:						// if requested frame-persistent dialog
			if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	// from a frame-persistent factory.
				goto CreateItPersistent;
			break;
			
		case XAP_DLGT_APP_PERSISTENT:						// if requested app-persistent dialog
			if (m_dialogType == XAP_DLGT_APP_PERSISTENT)		//   if from a app-persistent factory
				goto CreateItPersistent;
			if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   if from a frame-persistent factory,
				goto HandToAppFactory;						//     let the app's factory do it....
			break;
			
		case XAP_DLGT_MODELESS:						// if requested app-persistent dialog
			if (m_dialogType == XAP_DLGT_APP_PERSISTENT)		//   if from a app-persistent factory
				goto CreateItPersistent;
			if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   if from a frame-persistent factory,
				goto HandToAppFactory;						//     let the app's factory do it....
			break;

		}
	}

//	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;

CreateItSimple:
	{
		// create a fresh dialog object and return it -- no strings attached.
		
		pDialog = (XAP_Dialog *)((m_dlg_table[index].m_pfnStaticConstructor)(this,id));
		return pDialog;
	}
	
CreateItPersistent:
	{
		// see if we already have an instance of this object in our vector.
		// if so, just return it.  otherwise, create a fresh one and remember it.
		
		UT_sint32 indexVec = m_vecDialogIds.findItem((void*)(index+1));
		if (indexVec < 0)				// not present, create new object and add it to vector
		{
			pDialog = (XAP_Dialog *)((m_dlg_table[index].m_pfnStaticConstructor)(this,id));
			m_vecDialogIds.addItem((void*)(index+1));
			m_vecDialogs.addItem(pDialog);
		}
		else							// already present, reuse this object
		{
			pDialog = (XAP_Dialog *)m_vecDialogs.getNthItem(indexVec);
		}

		// let the dialog object know that we are reusing it.
		
		XAP_Dialog_Persistent * pDialogPersistent = (XAP_Dialog_Persistent *)pDialog;
		pDialogPersistent->useStart();
		
		return pDialog;
	}

HandToAppFactory:
	{
		// pass the request to the factory with the appropriate scope.
		
		UT_ASSERT(m_pFrame);
		pDialog = m_pFrame->getApp()->getDialogFactory()->requestDialog(id);
		return pDialog;
	}
}

void XAP_DialogFactory::releaseDialog(XAP_Dialog * pDialog)
{
	// the caller is done with the dialog.  if it is non-persistent, we
	// can just delete it.  otherwise, we should just store it for later
	// reuse.

	UT_ASSERT(pDialog);
	XAP_Dialog_Id id = pDialog->getDialogId();
	
	UT_uint32 index;
	_findDialogInTable(id,&index);

	switch (m_dlg_table[index].m_type)
	{
	case XAP_DLGT_NON_PERSISTENT:						// for non-persistent dialog objects, we
		delete pDialog;									// just delete it now.
		return;

	case XAP_DLGT_FRAME_PERSISTENT:						// if requested frame-persistent dialog
		if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   from a frame-persistent factory.
			goto FinishedUsingObject;					//     we remember it in our vector.
		break;
		
	case XAP_DLGT_APP_PERSISTENT:						// if requested app-persistent dialog
		if (m_dialogType == XAP_DLGT_APP_PERSISTENT)		//   if from a app-persistent factory
			goto FinishedUsingObject;					//     we remember it in our vector.
		if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   if from a frame-persistent factory,
			goto HandToAppFactory;						//     let the app's factory do it....
		break;
		
	case XAP_DLGT_MODELESS:						// if requested app-persistent dialog
		if (m_dialogType == XAP_DLGT_APP_PERSISTENT)		//   if from a app-persistent factory
			goto FinishedUsingObject;					//     we remember it in our vector.
		if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   if from a frame-persistent factory,
			goto HandToAppFactory;						//     let the app's factory do it....
		break;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return;

FinishedUsingObject:
	{
		// let the dialog object know that we are reusing it.
		
		XAP_Dialog_Persistent * pDialogPersistent = (XAP_Dialog_Persistent *)pDialog;
		pDialogPersistent->useEnd();
		return;
	}
	
HandToAppFactory:
	{
		// pass the request to the factory with the appropriate scope.
		
		UT_ASSERT(m_pFrame);
		m_pFrame->getApp()->getDialogFactory()->releaseDialog(pDialog);
		return;
	}
}


