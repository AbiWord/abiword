/* AbiSource Drag and Drop
 * Copyright (C) 2002 Jordi Mas i Hernàndez jmas@softcatala.org
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

#include <windows.h>
#include <richedit.h>


#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "ap_FrameData.h"
#include "ie_impexp_Register.h"
#include "ie_imp.h"
#include "xap_EncodingManager.h"
#include "xap_Win32DragAndDrop.h"
#include "fl_DocLayout.h"

XAP_Win32DropTarget::XAP_Win32DropTarget()
{
#define TEXT(z) L##z
	m_uCF_RTF = RegisterClipboardFormatW(CF_RTF);	
#undef  TEXT
	m_nCount = 0;
	
}

STDMETHODIMP XAP_Win32DropTarget::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	if (!ppvObj) return E_POINTER;
	if (riid==IID_IDropTarget) {
		*ppvObj=(IDropTarget*)this;
		return S_OK;
	} else if (riid==IID_IUnknown) {
		*ppvObj=(IUnknown*)this;
		return S_OK;
	}
	return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) XAP_Win32DropTarget::AddRef()
{
        ++m_nCount; 
        return S_OK;       
}


STDMETHODIMP_(ULONG) XAP_Win32DropTarget::Release()
{
        --m_nCount;
	return S_OK;
}

//
//      Called when the mouse first enters our DropTarget window
//	We check which formats we really support
//
STDMETHODIMP XAP_Win32DropTarget::DragEnter (LPDATAOBJECT pDataObj, DWORD /*grfKeyState*/, POINTL /*pointl*/, LPDWORD pdwEffect)
{	
	FORMATETC 	fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	
	m_bSupportedFormat = false;	

	// Dropping files
	if (NOERROR == pDataObj->QueryGetData(&fmte))	
		m_bSupportedFormat = true;				
	
	// RTF
	fmte.cfFormat = m_uCF_RTF;
	
	if (NOERROR == pDataObj->QueryGetData(&fmte))	
		m_bSupportedFormat = true;
	
	*pdwEffect = (m_bSupportedFormat) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		
	return S_OK;
}

STDMETHODIMP XAP_Win32DropTarget::DragOver(DWORD /*grfKeyState*/, POINTL /*pointl*/, LPDWORD pdwEffect)
{        	
	*pdwEffect = (m_bSupportedFormat) ? DROPEFFECT_COPY : DROPEFFECT_NONE;			
	
	SendMessageW(static_cast<XAP_Win32FrameImpl*>(m_pFrame->getFrameImpl())->getTopLevelWindow(), WM_VSCROLL, SB_LINEDOWN, 0);

	return S_OK;
}


STDMETHODIMP XAP_Win32DropTarget::DragLeave()
{
	return S_OK;
}


//
// Called when the user drops an object
//
STDMETHODIMP XAP_Win32DropTarget::Drop (LPDATAOBJECT pDataObj, DWORD /*grfKeyState*/, POINTL /*pointl*/, LPDWORD /*pdwEffect*/)
{
        
	FORMATETC  	formatetc;
	STGMEDIUM	medium;
	char*		pData;	
	UT_uint32 	iStrLen;
	IE_Imp* 	pImp = 0;
	const char * 	szEncoding = 0;
	int 		count = 0;
	
	FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	
	//
	// Is the user dropping files?
	//
	if (NOERROR == pDataObj->QueryGetData(&fmte))
	{
		if (pDataObj && SUCCEEDED (pDataObj->GetData (&fmte, &medium)))
			count = DragQueryFileW((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);
		
		// We send an event Message Window to the Win32Frame since historicaly
		// the file loading was processed there
		if (count) 
			SendMessageW(static_cast<XAP_Win32FrameImpl*>(m_pFrame->getFrameImpl())->getTopLevelWindow(), WM_DROPFILES, (WPARAM)medium.hGlobal, 0);
				
		ReleaseStgMedium(&medium);	     
		return S_OK;
	}
	

	//
	// Is the user dropping RTF text?
	//				
	formatetc.cfFormat = m_uCF_RTF;
	formatetc.ptd = NULL;
	formatetc.dwAspect  = DVASPECT_CONTENT;
	formatetc.lindex  = -1;
	formatetc.tymed = TYMED_HGLOBAL;
	
	medium.hGlobal = NULL;	
	medium.pUnkForRelease = NULL;	
            	
      	// Does not support RTF
        if (!SUCCEEDED(pDataObj->GetData(&formatetc, &medium)))
	       	return S_OK;
        
	pData = (char *) GlobalLock (medium.hGlobal);                             		
	iStrLen =  strlen(pData);
	
	// Get document range
	AP_FrameData* pFrameData = (AP_FrameData*) m_pFrame->getFrameData();
	FL_DocLayout *pDocLy =	pFrameData->m_pDocLayout;
	FV_View * pView =  pDocLy->getView();	
	PD_DocumentRange dr(pView->getDocument(),pView->getPoint(),pView->getPoint());
			
	// Import RTF	
	IE_Imp::constructImporter(dr.m_pDoc, IE_Imp::fileTypeForSuffix(".rtf"), &pImp, 0);
	if (pImp)
	{		
		szEncoding = XAP_EncodingManager::get_instance()->getNative8BitEncodingName();		
						
		pImp->pasteFromBuffer(&dr, (unsigned char *)pData, iStrLen,szEncoding);
		
		delete pImp;		
		pView->_generalUpdate();
	}
	
	GlobalUnlock(medium.hGlobal);
	GlobalFree(medium.hGlobal);
	ReleaseStgMedium(&medium);	     	
                
        return S_OK;
}




