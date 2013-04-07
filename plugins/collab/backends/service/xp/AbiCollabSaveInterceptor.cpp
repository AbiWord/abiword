/* Copyright (C) 2008-2009 AbiSource Corporation B.V.
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

#include <boost/bind.hpp>
#include "xap_App.h"
#include "ev_Menu_Actions.h"
#include "ap_Menu_Id.h"
#include "ap_Menu_Functions.h"
#include "ev_Toolbar_Actions.h"
#include "ap_Toolbar_Id.h"
#include "ap_Toolbar_Functions.h"
#include "ap_LoadBindings.h"
#include "ev_EditEventMapper.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "xap_Frame.h"
#include "ut_debugmsg.h"
#include "AsyncWorker.h"

#include "ServiceAccountHandler.h"
#include <core/session/xp/AbiCollab.h>
#include <core/session/xp/AbiCollabSessionManager.h>
#include "AbiCollabSaveInterceptor.h"
#include "soa_soup.h"

#define SAVE_INTERCEPTOR_EM "com.abisource.abiword.abicollab.servicesaveinterceptor"

static bool AbiCollabSaveInterceptor_interceptor(AV_View * v, EV_EditMethodCallData * d)
{
	return ServiceAccountHandler::m_saveInterceptor.intercept(v, d);
}

#define DO_NOT_USE ""

static ap_bs_Char CharTable[] =
{
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
	{0x53, /* S      */ { "insertData",			"fileSaveAs", 		DO_NOT_USE,		""					}},
	{0x73, /* s      */ { "insertData",			SAVE_INTERCEPTOR_EM,		    DO_NOT_USE,		""					}},
};

AbiCollabSaveInterceptor::AbiCollabSaveInterceptor()
	: m_pOldSaveEM(NULL)
{
	UT_DEBUGMSG(("Installing Save menu interceptor!\n"));
	EV_EditMethodContainer *pEMC = XAP_App::getApp()->getEditMethodContainer();

	// store the old/normale editmethod to fall back when a document is not under service control
	m_pOldSaveEM = pEMC->findEditMethodByName("fileSave");

	UT_return_if_fail(m_pOldSaveEM);
	// install the edit method we will use to save to the webapp
	EV_EditMethod* mySaveInterceptor = new EV_EditMethod (
								SAVE_INTERCEPTOR_EM, 
								&AbiCollabSaveInterceptor_interceptor, 
								0, "AbiCollab Service Save Interceptor"
							);
	pEMC->addEditMethod(mySaveInterceptor);	
	
	// install the new menu action with our custom save edit method
	XAP_App::getApp()->getMenuActionSet()->setAction(
			AP_MENU_ID_FILE_SAVE,
			false, /* holds submenu */
			false, /* raises dialog */
			false, /* is checkable */
			false, /* is radio */
			SAVE_INTERCEPTOR_EM,
			ap_GetState_Changes,
			NULL, /* state label */
			NULL
		);
	
	// install the new toolbar action with our custom save edit method
	XAP_App::getApp()->getToolbarActionSet()->setAction(
			AP_TOOLBAR_ID_FILE_SAVE,
			EV_TBIT_PushButton,
			SAVE_INTERCEPTOR_EM,
			AV_CHG_ALL,
			ap_ToolbarGetState_Changes
		);

	// install the new CTRL-s hook
	// TODO: what to the with Save As?
	const char * szCurrMode = XAP_App::getApp()->getInputMode();
	EV_EditBindingMap* pEbMap = XAP_App::getApp()->getBindingMap(szCurrMode);
	UT_return_if_fail(pEbMap);
		
	AP_BindingSet* pBindingSet = static_cast<AP_BindingSet*>(XAP_App::getApp()->getBindingSet());
	UT_return_if_fail(pBindingSet);

	pBindingSet->_loadChar(pEbMap, CharTable, 2, NULL, 0);	
}

bool AbiCollabSaveInterceptor::intercept(AV_View * v, EV_EditMethodCallData * d)
{
	UT_DEBUGMSG(("AbiCollabSaveInterceptor_intercept\n"));
	UT_return_val_if_fail(v, false);
	FV_View* pView = static_cast<FV_View*>(v);
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	PD_Document* pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);
	
	if (!pDoc->isDirty())
	{
		UT_DEBUGMSG(("Document is not dirty, not saving.\n"));
		return true;
	}

	if (!pManager->isInSession(pDoc))
		return m_pOldSaveEM->Fn(v, d);

	UT_DEBUGMSG(("Document is in a collaboration session!\n"));
	AbiCollab* pSession = pManager->getSession(pDoc);
	UT_return_val_if_fail(pSession, m_pOldSaveEM->Fn(v, d));

	if (save(pDoc))
	{
	    XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	    if (pFrame->getViewNumber() > 0)
	      XAP_App::getApp()->updateClones(pFrame);
	    return true;
	}
	UT_DEBUGMSG(("This session does not use the abicollab webservice; saving the old fashioned way...\n"));
	return m_pOldSaveEM->Fn(v, d);
}

bool AbiCollabSaveInterceptor::save(PD_Document* pDoc)
{
	UT_DEBUGMSG(("AbiCollabSaveInterceptor::save()\n"));
	UT_return_val_if_fail(pDoc, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	AbiCollab* pSession = pManager->getSession(pDoc);
	UT_return_val_if_fail(pSession, false);

	// the session id should be unique on a specific account handler; let's 
	// just look it up amongst all our account handlers (not too efficient or
	// elegant, but it works)
	for (UT_uint32 i = 0; i < pManager->getAccounts().size(); i++)
	{
		AccountHandler* pHandler = pManager->getAccounts()[i];
		UT_continue_if_fail(pHandler);
		if (pHandler->getStorageType() == SERVICE_ACCOUNT_HANDLER_TYPE)
		{
			ServiceAccountHandler* pServiceHandler = static_cast<ServiceAccountHandler*>(pHandler);
			ConnectionPtr connection_ptr = pServiceHandler->getConnection(pDoc);
			if (!connection_ptr)
				continue; // apparently we need another abicollab account
			UT_DEBUGMSG(("Found the abicollab webservice account handler (%s) that controls this session!\n", pServiceHandler->getDescription().utf8_str()));

			pManager->beginAsyncOperation(pSession);
			// FIXME: guarantee save order!
			
			std::string uri = pServiceHandler->getProperty("uri");
			bool verify_webapp_host = (pServiceHandler->getProperty("verify-webapp-host") == "true");
			soa::function_call_ptr fc_ptr = pServiceHandler->constructSaveDocumentCall(pDoc, connection_ptr);
			std::string ssl_ca_file = pServiceHandler->getCA();
			boost::shared_ptr<std::string> result_ptr(new std::string());
			boost::shared_ptr<AsyncWorker<bool> > async_save_ptr(
						new AsyncWorker<bool>(
							boost::bind(&AbiCollabSaveInterceptor::_save, this, uri, verify_webapp_host, ssl_ca_file, fc_ptr, result_ptr),
							boost::bind(&AbiCollabSaveInterceptor::_save_cb, this, _1, pServiceHandler, pSession, connection_ptr, fc_ptr, result_ptr)
						)
					);
			async_save_ptr->start();
			
			// make the document clean (even if it isn't _yet_)
			pDoc->setClean();
			pDoc->signalListeners(PD_SIGNAL_DOCNAME_CHANGED);
			return true;
		}
	}

	return false;
}


// NOTE: don't use const std::string& arguments where, as we want a copy for thread safety
bool AbiCollabSaveInterceptor::_save(std::string uri, bool verify_webapp_host, std::string ssl_ca_file,
			soa::function_call_ptr fc_ptr, boost::shared_ptr<std::string> result_ptr)
{
	UT_DEBUGMSG(("AbiCollabSaveInterceptor::_save()\n"));
	UT_return_val_if_fail(fc_ptr, false);
	UT_return_val_if_fail(result_ptr, false);

	// execute the call
	bool res = soup_soa::invoke(uri, soa::method_invocation("urn:AbiCollabSOAP", *fc_ptr), verify_webapp_host?ssl_ca_file:"", *result_ptr);
	UT_return_val_if_fail(res, false);

	UT_DEBUGMSG(("Document uploaded successfully\n"));
	return true;
}

void AbiCollabSaveInterceptor::_save_cb(bool success, ServiceAccountHandler* pAccount,
			AbiCollab* pSession, ConnectionPtr connection_ptr,
			soa::function_call_ptr fc_ptr, boost::shared_ptr<std::string> result_ptr)
{
	UT_DEBUGMSG(("AbiCollabSaveInterceptor::_save_cb()\n"));
	UT_return_if_fail(pAccount);
	UT_return_if_fail(pSession);
	UT_return_if_fail(connection_ptr);
	UT_return_if_fail(fc_ptr);
	UT_return_if_fail(result_ptr);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);	

	if (success)
	{
		// parse the soap result
		try {
			// we ignore the result (the new revision number), we're happy when there were no errors
			soa::method_invocation mi("urn:AbiCollabSOAP", *fc_ptr);
			soa::GenericPtr soap_result = soa::parse_response(*result_ptr, mi.function().response());
			if (soap_result)
			{
				// the save was successful, so we can mark this async action as completed
				pManager->endAsyncOperation(pSession);
				return;
			}
		} catch (soa::SoapFault& fault) {
			UT_DEBUGMSG(("Caught a soap fault: %s (error code: %s)!\n", 
					 fault.detail() ? fault.detail()->value().c_str() : "(null)",
					 fault.string() ? fault.string()->value().c_str() : "(null)"));

			acs::SOAP_ERROR err = acs::error(fault);
			switch (err)
			{
				case acs::SOAP_ERROR_INVALID_PASSWORD:
				{
					const std::string email = pAccount->getProperty("email");
					std::string uri = pAccount->getProperty("uri");
					bool verify_webapp_host = (pAccount->getProperty("verify-webapp-host") == "true");
					std::string ssl_ca_file = pAccount->getCA();

					std::string password;
					if (ServiceAccountHandler::askPassword(email, password))
					{
						// store the new password
						pAccount->addProperty("password", password);
						pManager->storeProfile();

						// construct a SOAP call with the new password
						// FIXME: this is highly ineffictient as it serializes the whole document again
						fc_ptr = pAccount->constructSaveDocumentCall(pSession->getDocument(), connection_ptr);

						// re-attempt the save
						boost::shared_ptr<AsyncWorker<bool> > async_save_ptr(
									new AsyncWorker<bool>(
										boost::bind(&AbiCollabSaveInterceptor::_save, this, uri, verify_webapp_host, ssl_ca_file, fc_ptr, result_ptr),
										boost::bind(&AbiCollabSaveInterceptor::_save_cb, this, _1, pAccount, pSession, connection_ptr, fc_ptr, result_ptr)
									)
								);
						async_save_ptr->start();

						// we re-attempt the save, so don't mark this async action as completed
						// before exiting this function
						return;
					}
				}
				case acs::SOAP_ERROR_NO_CHANGES:
					// the save was successful (unneeded), so we can mark this async action as completed
					pManager->endAsyncOperation(pSession);
					UT_DEBUGMSG(("The document was unchanged; ignoring error\n"));
					return;
				default:
					break;
			}
		}
	}

	// the save failed and we can't recover from it, so we can mark this async action as completed
	pManager->endAsyncOperation(pSession);
	// inform the user of the failure to save
	_saveFailed(pSession);
}

void AbiCollabSaveInterceptor::_saveFailed(AbiCollab* pSession)
{
	// WARNING: do NOT assume we have a valid view or frame here: it could already 
	// have been deleted if the frame was closed (or abiword shutdown) before this 
	// callback came back.
	// You can safely use the AbiCollab pointer or PD_Document pointer though, as
	// the AbiCollabSessionManager makes sure those are still valid.

	UT_return_if_fail(pSession);

	PD_Document* pDoc = pSession->getDocument();
	UT_return_if_fail(pDoc);

	// the document was not saved after all, mark it dirty again
	pDoc->forceDirty();
	pDoc->signalListeners(PD_SIGNAL_DOCNAME_CHANGED);

	// idealy we would use the same frame that was used to save the document,
	// but we don't know if that one is still valid
	if (XAP_App::getApp()->getLastFocussedFrame())
	{
		// TODO: add the document name, error type and perhaps the server name
		// TODO: offer some kind of solution to the user
		UT_UTF8String msg("An error occured while saving this document to the web-service!");
		XAP_App::getApp()->getLastFocussedFrame()->showMessageBox(msg.utf8_str(), XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
	}
}