/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_debugmsg.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"

// save some typing
typedef std::multimap<XAP_Dialog_Id, const XAP_NotebookDialog::Page*> NotebookPages;
typedef NotebookPages::iterator NotebookPagesIter;

static NotebookPages s_mapNotebookPages;

/*****************************************************************/

XAP_DialogFactory::XAP_DialogFactory(XAP_App * pApp, int nrElem, const struct _dlg_table * pDlgTable, XAP_Frame * pFrame)
  : m_pApp(pApp),
	m_pFrame(pFrame),
	m_dialogType(XAP_DLGT_APP_PERSISTENT),
	m_nrElementsDlgTable(nrElem)
{
	UT_ASSERT(pApp);
	UT_sint32 i;
	
	for (i = 0; i < nrElem; i++)
	{
		m_vec_dlg_table.addItem(&pDlgTable[i]);
	}
	
#ifdef DEBUG
	// getNextId() assumes that the last item is the greater. We should
	//  keep greater ID always the latest on ap_[PLATFORM]Dialog_All.h	
	UT_sint32 greatest = 0;
	for(i=0; i< nrElem; i++)
	{
		if (pDlgTable[i].m_id > greatest)
			greatest = pDlgTable[i].m_id;
	}
	
	if (nrElem)
		UT_ASSERT(greatest == pDlgTable[nrElem - 1].m_id);
#endif

}

XAP_DialogFactory::~XAP_DialogFactory(void)
{
	UT_VECTOR_PURGEALL(XAP_Dialog *, m_vecDialogs);
	UT_VECTOR_PURGEALL( _dlg_table *, m_vecDynamicTable);
}

bool XAP_DialogFactory::_findDialogInTable(XAP_Dialog_Id id, UT_sint32 * pIndex) const
{
	// search the table and return the index of the entry with this id.
	UT_return_val_if_fail(pIndex, false);

	for (UT_sint32 k=0; k < m_vec_dlg_table.getItemCount(); k++)
	{
		if (m_vec_dlg_table.getNthItem(k)->m_id == id)
		{
			*pIndex = k;
			return true;
		}
	}
	UT_DEBUGMSG(("Could not find a match for id %d \n",id));
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return false;
}

XAP_Dialog_Id XAP_DialogFactory::getNextId(void)
{
       UT_sint32 i = m_vec_dlg_table.getItemCount()-1;
	UT_sint32 id = static_cast<UT_sint32>(m_vec_dlg_table.getNthItem(i)->m_id);
	return static_cast<XAP_Dialog_Id>(id+1);
}

XAP_Dialog_Id XAP_DialogFactory::registerDialog(XAP_Dialog *(* pStaticConstructor)(XAP_DialogFactory *, XAP_Dialog_Id id),XAP_Dialog_Type iDialogType)
{
	_dlg_table * pDlgTable = new _dlg_table;
	pDlgTable->m_id = getNextId();
	pDlgTable->m_type = iDialogType;
	pDlgTable->m_pfnStaticConstructor = pStaticConstructor;
	pDlgTable->m_tabbed = FALSE;
	m_vec_dlg_table.addItem(pDlgTable);
	m_vecDynamicTable.addItem(pDlgTable);
	return pDlgTable->m_id;
}

void XAP_DialogFactory::unregisterDialog(XAP_Dialog_Id id)
{
	for (UT_sint32 i = 0; i < m_vecDialogs.getItemCount(); i++)
	{
		const XAP_Dialog * pDialog = reinterpret_cast<const XAP_Dialog *>(m_vecDialogs.getNthItem(i));
		if(pDialog && pDialog->getDialogId() == id)
		{
			m_vecDialogs.deleteNthItem(i);
			m_vecDialogIds.deleteNthItem(i);
			delete pDialog;
			return;
		}
	}
}

/*****************************************************************/

/*!
 * This method just creates a new instance of the dialog without remembering
 * anything about it. It's up the to calling prgram to delete the dialog 
 * when it is finished with it.
\param XAP_Dialog_Id id the identification number of the dialog.
\returns XAP_Dialog * pointer to the new instance of the dialog.
 */
XAP_Dialog * XAP_DialogFactory::justMakeTheDialog(XAP_Dialog_Id id)
{
	UT_sint32 index;
	XAP_Dialog * pDialog = NULL;
	
	if(_findDialogInTable(id,&index))
	{
	  pDialog = (XAP_Dialog *)((m_vec_dlg_table.getNthItem(index)->m_pfnStaticConstructor)(this,id));
		return pDialog;
	}
	return NULL;
}

XAP_Dialog * XAP_DialogFactory::requestDialog(XAP_Dialog_Id id)
{
	const _dlg_table * dlg = NULL;
	XAP_Dialog * pDialog = NULL;
	UT_sint32 index;
	
	if(_findDialogInTable(id, &index))
	{
		dlg = m_vec_dlg_table.getNthItem(index);
		switch (dlg->m_type)
		{
		case XAP_DLGT_NON_PERSISTENT:	
			// construct a non-persistent dialog and just return it.
			goto CreateItSimple;

		case XAP_DLGT_FRAME_PERSISTENT:	 // if requested frame-persistent dialog
			if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	// from a frame-persistent factory.
				goto CreateItPersistent;
			break;
			
		case XAP_DLGT_APP_PERSISTENT:	// if requested app-persistent dialog
			if (m_dialogType == XAP_DLGT_APP_PERSISTENT)  //   if from a app-persistent factory
				goto CreateItPersistent;
			if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   if from a frame-persistent factory,
				goto HandToAppFactory;	 //     let the app's factory do it....
			break;
			
		case XAP_DLGT_MODELESS:						// if requested app-persistent dialog
			if (m_dialogType == XAP_DLGT_APP_PERSISTENT)		//   if from a app-persistent factory
				goto CreateItPersistent;
			if (m_dialogType == XAP_DLGT_FRAME_PERSISTENT)	//   if from a frame-persistent factory,
				goto HandToAppFactory;						//     let the app's factory do it....
			break;

		}
	}

//	UT_ASSERT_NOT_REACHED();
	return NULL;

CreateItSimple:
	{
		// create a fresh dialog object and return it -- no strings attached.
		pDialog = (XAP_Dialog *)((dlg->m_pfnStaticConstructor)(this,id));
		if (dlg->m_tabbed) {
			XAP_NotebookDialog * d = dynamic_cast<XAP_NotebookDialog *>(pDialog);
			UT_ASSERT(d);
			addPages(d, id);
		}
		return pDialog;
	}
	
CreateItPersistent:
	{
		// see if we already have an instance of this object in our vector.
		// if so, just return it.  otherwise, create a fresh one and remember it.
		UT_sint32 indexVec = m_vecDialogIds.findItem(index+1);
		if (indexVec < 0)				// not present, create new object and add it to vector
		{
			pDialog = (XAP_Dialog *)((dlg->m_pfnStaticConstructor)(this,id));
			m_vecDialogIds.addItem(index+1);
			m_vecDialogs.addItem(pDialog);
		}
		else							// already present, reuse this object
		{
			pDialog = (XAP_Dialog *)m_vecDialogs.getNthItem(indexVec);
		}
		if (dlg->m_tabbed) {
			XAP_NotebookDialog * d = dynamic_cast<XAP_NotebookDialog *>(pDialog);
			UT_ASSERT(d);
			addPages(d, id);
		}

		// let the dialog object know that we are reusing it.
		
		XAP_Dialog_Persistent * pDialogPersistent = (XAP_Dialog_Persistent *)pDialog;
		pDialogPersistent->useStart();
		
		return pDialog;
	}

HandToAppFactory:
	{
		// pass the request to the factory with the appropriate scope.
		pDialog = XAP_App::getApp()->getDialogFactory()->requestDialog(id);
		return pDialog;
	}
}

void XAP_DialogFactory::releaseDialog(XAP_Dialog * pDialog)
{
	// the caller is done with the dialog.  if it is non-persistent, we
	// can just delete it.  otherwise, we should just store it for later
	// reuse.

	UT_return_if_fail(pDialog);
	XAP_Dialog_Id id = pDialog->getDialogId();
	
	UT_sint32 index;
	_findDialogInTable(id,&index);

	switch (m_vec_dlg_table.getNthItem(index)->m_type)
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

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
		XAP_App::getApp()->getDialogFactory()->releaseDialog(pDialog);
		return;
	}
}

/*!
 * Add a notebook page to a builtin dialog.
 */
bool XAP_DialogFactory::registerNotebookPage(XAP_Dialog_Id dialog, const XAP_NotebookDialog::Page * page)
{
	// check that widget is unique for dialog
	std::pair<NotebookPagesIter, NotebookPagesIter> bounds = s_mapNotebookPages.equal_range(dialog);
	while (bounds.first != bounds.second)
	{
		if (bounds.first->second == page)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return FALSE;
		}
		bounds.first++;
	}

	s_mapNotebookPages.insert(NotebookPages::value_type(dialog, page));
	return TRUE;
}

/*!
 * Remove a previously added page from a dialog.
 */
bool XAP_DialogFactory::unregisterNotebookPage(XAP_Dialog_Id dialog, const XAP_NotebookDialog::Page * page)
{
	std::pair<NotebookPagesIter, NotebookPagesIter> bounds = s_mapNotebookPages.equal_range(dialog);
	while (bounds.first != bounds.second)
	{
		// widget per dialog must be unique, that's made sure in registerNotebookPage.
		if (bounds.first->second == page)
		{
			s_mapNotebookPages.erase(bounds.first);
			return TRUE;
		}
		bounds.first++;
	}
	return FALSE;
}

/*!
 * Add registered pages to the dialog instance.
 */
void XAP_DialogFactory::addPages(XAP_NotebookDialog * pDialog, XAP_Dialog_Id id)
{
	std::pair<NotebookPagesIter, NotebookPagesIter> bounds = s_mapNotebookPages.equal_range(id);
	while (bounds.first != bounds.second)
	{
		pDialog->addPage(bounds.first->second);
		bounds.first++;
	}
}
