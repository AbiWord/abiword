/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2008 AbiSource Corporation B.V.
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

#ifndef WIN32
#include <unistd.h>
#endif
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"
#include "ut_debugmsg.h"
#include "ut_sleep.h"
#include "soa_soup.h"
#include "abicollab_types.h"
#include "AsyncWorker.h"
#include "ProgressiveSoapCall.h"
#include "RealmBuddy.h"
#include "ServiceBuddy.h"
#include "ServiceAccountHandler.h"
#include "ap_Dialog_GenericInput.h"
#include <backends/xp/SessionEvent.h>
#include <core/session/xp/AbiCollabSessionManager.h>
#include "AbiCollabService_Export.h"

namespace rpv1 = realm::protocolv1;

XAP_Dialog_Id ServiceAccountHandler::m_iDialogGenericInput = 0;
XAP_Dialog_Id ServiceAccountHandler::m_iDialogGenericProgress = 0;
AbiCollabSaveInterceptor ServiceAccountHandler::m_saveInterceptor = AbiCollabSaveInterceptor();

bool ServiceAccountHandler::askPassword(const std::string& email, std::string& password)
{
	UT_DEBUGMSG(("ServiceAccountHandler::askPassword()\n"));
	XAP_Frame* pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, false);
	
	// ask for the service password
	XAP_DialogFactory* pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	UT_return_val_if_fail(pFactory, false);
	AP_Dialog_GenericInput* pDialog = static_cast<AP_Dialog_GenericInput*>(
				pFactory->requestDialog(ServiceAccountHandler::getDialogGenericInputId())
			);
	
	// Run the dialog
	// TODO: make this translatable
	pDialog->setTitle("AbiCollab.net Collaboration Service"); // FIXME: don't hardcode this title to abicollab.net
	std::string msg = "Please enter your password for account '" + email + "'";
	pDialog->setQuestion(msg.c_str());
	pDialog->setLabel("Password:");
	pDialog->setPassword(true);
	pDialog->runModal(pFrame);
	
	// get the results
	bool cancel = pDialog->getAnswer() == AP_Dialog_GenericInput::a_CANCEL;
	if (!cancel)
		password = pDialog->getInput().utf8_str();
	pFactory->releaseDialog(pDialog);
	
	// the user terminated the input
	return !cancel;
}

ServiceAccountHandler::ServiceAccountHandler()
	: AccountHandler(),
	m_bOnline(false),
	  m_connections(),
	  m_iListenerID(0),
	  m_pExport(NULL)
{
	m_ssl_ca_file = XAP_App::getApp()->getAbiSuiteLibDir();
#if defined(WIN32)
	m_ssl_ca_file += "\\certs\\cacert.pem";
#else
	m_ssl_ca_file += "/certs/cacert.pem";
#endif
}

ServiceAccountHandler::~ServiceAccountHandler()
{
	disconnect();
}

UT_UTF8String ServiceAccountHandler::getDescription()
{
    return getProperty("email").c_str();
}

UT_UTF8String ServiceAccountHandler::getDisplayType()
{
	return "AbiCollab.net Collaboration Service";
}

UT_UTF8String ServiceAccountHandler::getStorageType()
{
	return SERVICE_ACCOUNT_HANDLER_TYPE;
}

void ServiceAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("ServiceAccountHandler::storeProperties() - TODO: implement me\n"));
}

XAP_Dialog_Id ServiceAccountHandler::getDialogGenericInputId()
{
	// register the generic input dialog if we haven't already done that
	// a bit hacky, but it works
	if (m_iDialogGenericInput == 0)
	{
		XAP_DialogFactory * pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
		m_iDialogGenericInput = pFactory->registerDialog(ap_Dialog_GenericInput_Constructor, XAP_DLGT_NON_PERSISTENT);
	}
	return m_iDialogGenericInput;
}

XAP_Dialog_Id ServiceAccountHandler::getDialogGenericProgressId()
{
	// register the generic progress dialog if we haven't already done that
	// a bit hacky, but it works
	if (m_iDialogGenericProgress == 0)
	{
		XAP_DialogFactory * pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
		m_iDialogGenericProgress = pFactory->registerDialog(ap_Dialog_GenericProgress_Constructor, XAP_DLGT_NON_PERSISTENT);
	}
	return m_iDialogGenericProgress;
}

ConnectResult ServiceAccountHandler::connect()
{
	UT_DEBUGMSG(("ServiceAccountHandler::connect()\n"));
	
	if (m_bOnline)
		return CONNECT_SUCCESS;
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, CONNECT_INTERNAL_ERROR);	
	
	m_bOnline = true;

	// we are "connected" now, time to start sending out, and listening to messages (such as events)
	pManager->registerEventListener(this);

	return CONNECT_SUCCESS;
}

bool ServiceAccountHandler::disconnect()
{
	UT_DEBUGMSG(("ServiceAccountHandler::disconnect()\n"));
	UT_return_val_if_fail(m_bOnline, false);
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);	
	
	m_bOnline = false;
	
	// TODO: send out an event we are going offline
	// ...
	
	// we are disconnected now, no need to sent out messages (such as events) anymore
	pManager->unregisterEventListener(this);	
	removeExporter();
	return true;
}

void ServiceAccountHandler::removeExporter(void)
{
	if(m_pExport)
        {
	    PD_Document * pDoc = m_pExport->getDocument();
	    pDoc->removeListener(m_iListenerID);
	    m_iListenerID = 0;
	    DELETEP(m_pExport);
	}
}

bool ServiceAccountHandler::isOnline()
{
	return m_bOnline;
}

Buddy* ServiceAccountHandler::constructBuddy(const PropertyMap& props)
{
	UT_DEBUGMSG(("ServiceAccountHandler::constructBuddy() - TODO: implement me\n"));
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);	
	return NULL; // TODO: implement me
}

bool ServiceAccountHandler::send(const Packet* packet)
{
	UT_DEBUGMSG(("ServiceAccountHandler::send(const Packet*)\n"));
	// this are typically announce session broadcast events and the like,
	// which we don't support in this backend
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);	
	return true;
}

bool ServiceAccountHandler::send(const Packet* packet, const Buddy& buddy)
{
	UT_DEBUGMSG(("ServiceAccountHandler::send(const Packet*, const Buddy& buddy)\n"));

	const RealmBuddy& ourBuddy = static_cast<const RealmBuddy&>(buddy);
	uint8_t arr[] = { ourBuddy.realm_connection_id() };
	std::vector<uint8_t> connection_ids(arr, arr+1);

	boost::shared_ptr<std::string> data(new std::string());
	_createPacketStream( *data, packet );

	_send(boost::shared_ptr<rpv1::RoutingPacket>(new rpv1::RoutingPacket(connection_ids, data)), ourBuddy.ptr());
	return true;
}

void ServiceAccountHandler::_write_handler(const asio::error_code& e, std::size_t bytes_transferred,
											boost::shared_ptr<const RealmBuddy> recipient, boost::shared_ptr<realm::protocolv1::Packet> packet)
{
	if (e)
	{
		// TODO: disconnect buddy
		UT_DEBUGMSG(("Error sending packet: %s\n", e.message().c_str()));
		return;
	}
	
	UT_DEBUGMSG(("Packet sent: 0x%x\n", packet->type()));
}										   

void ServiceAccountHandler::getSessionsAsync()
{
	UT_DEBUGMSG(("ServiceAccountHandler::getSessionsAsync()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);			
	
	pManager->beginAsyncOperation(this);
	SessionBuddyPtr sessions_ptr(new std::vector<SessionBuddyPair>());
	boost::shared_ptr<AsyncWorker<acs::SOAP_ERROR> > async_list_docs_ptr(
				new AsyncWorker<acs::SOAP_ERROR>(
					boost::bind(&ServiceAccountHandler::_listDocuments, this, 
								getProperty("uri"), getProperty("email"), getProperty("password"), sessions_ptr),
					boost::bind(&ServiceAccountHandler::_listDocuments_cb, this, _1, sessions_ptr)
				)
			);
	async_list_docs_ptr->start();	
}

void ServiceAccountHandler::getSessionsAsync(const Buddy& buddy)
{
	UT_DEBUGMSG(("ServiceAccountHandler::getSessionsAsync(const Buddy& buddy)\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);		
	
	// TODO: we shouldn't ignore the buddy parameter, but for now, we do ;)
	
	pManager->beginAsyncOperation(this);
	SessionBuddyPtr sessions_ptr(new std::vector<SessionBuddyPair>());
	boost::shared_ptr<AsyncWorker<acs::SOAP_ERROR> > async_list_docs_ptr(
				new AsyncWorker<acs::SOAP_ERROR>(
					boost::bind(&ServiceAccountHandler::_listDocuments, this, 
								getProperty("uri"), getProperty("email"), getProperty("password"), sessions_ptr),
					boost::bind(&ServiceAccountHandler::_listDocuments_cb, this, _1, sessions_ptr)
				)
			);
	async_list_docs_ptr->start();	
}

bool ServiceAccountHandler::hasSession(const UT_UTF8String& sSessionId)
{
	for (std::vector<boost::shared_ptr<RealmConnection> >::iterator it = m_connections.begin(); it != m_connections.end(); it++)
	{
		boost::shared_ptr<RealmConnection> connection_ptr = *it;
		UT_continue_if_fail(connection_ptr);
		if (connection_ptr->session_id() == sSessionId.utf8_str())
			return true;
	}
	return AccountHandler::hasSession(sSessionId);
}

// NOTE: we don't implement the opening of documents asynchronous; we block on it,
// as it's annoying to opening them async. We need to change this API
void ServiceAccountHandler::joinSessionAsync(const Buddy& buddy, DocHandle& docHandle)
{
	UT_DEBUGMSG(("ServiceAccountHandler::getSessionsAsync(const Buddy& buddy, DocHandle& docHandle)\n"));
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);		
	
	UT_DEBUGMSG(("Joining document %s\n", docHandle.getSessionId().utf8_str()));
	
	UT_sint64 doc_id;
	try {
		doc_id = boost::lexical_cast<int64_t>(docHandle.getSessionId().utf8_str());
	} catch (boost::bad_lexical_cast &) {
		// TODO: report error
		UT_DEBUGMSG(("Error casting doc_id (%s) to an UT_sint64\n", docHandle.getSessionId().utf8_str()));
		return;
	}
	UT_return_if_fail(doc_id != 0);
	UT_DEBUGMSG(("doc_id: %lld\n", doc_id));	

	PD_Document* pDoc = NULL;
	acs::SOAP_ERROR err = openDocument(doc_id, 0, docHandle.getSessionId().utf8_str(), &pDoc, NULL);
	switch (err)
	{
		case acs::SOAP_ERROR_OK:
			return;
		case acs::SOAP_ERROR_INVALID_PASSWORD:
			{
				// TODO: asking for user input is not really nice in an async function
				const std::string email = getProperty("email");
				std::string password;
				if (askPassword(email, password))
				{
					// try again with the new password
					addProperty("password", password);
					pManager->storeProfile();
					joinSessionAsync(buddy, docHandle);				
				}
			}
			return;
		default:
			{
				// TODO: add the document name, error type and perhaps the server name
				UT_UTF8String msg("Error importing document ");
				msg += docHandle.getName();
				msg += ".";
				XAP_App::getApp()->getLastFocussedFrame()->showMessageBox(msg.utf8_str(), XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
			}
			break;
	}
}

acs::SOAP_ERROR ServiceAccountHandler::openDocument(UT_sint64 doc_id, UT_sint64 revision, const std::string& session_id, PD_Document** pDoc, XAP_Frame* pFrame)
{
	UT_DEBUGMSG(("ServiceAccountHandler::openDocument() - doc_id: %lld\n", doc_id));
	
	const std::string uri = getProperty("uri");
	const std::string email = getProperty("email");
	const std::string password = getProperty("password");
		
	// construct a SOAP method call to gets our documents
	soa::function_call fc("openDocument", "openDocumentResponse");
	fc("email", email)("password", password)("doc_id", doc_id)("revision", static_cast<int64_t>(revision));

	// execute the call
	boost::shared_ptr<ProgressiveSoapCall> call(new ProgressiveSoapCall(uri, fc, m_ssl_ca_file));
	soa::GenericPtr soap_result;
	try {
		soap_result = call->run();
	} catch (soa::SoapFault& fault) {
		UT_DEBUGMSG(("Caught a soap fault: %s (error code: %s)!\n", 
					 fault.detail() ? fault.detail()->value().c_str() : "(null)",
					 fault.string() ? fault.string()->value().c_str() : "(null)"));
		return acs::error(fault);
	}
	UT_return_val_if_fail(soap_result, acs::SOAP_ERROR_GENERIC);
	
	// handle the result
	soa::CollectionPtr rcp = soap_result->as<soa::Collection>("return");
	UT_return_val_if_fail(rcp, acs::SOAP_ERROR_GENERIC);

	soa::StringPtr realm_address = rcp->get<soa::String>("realm_address");
	soa::IntPtr realm_port = rcp->get<soa::Int>("realm_port");
	soa::StringPtr cookie = rcp->get<soa::String>("cookie");
	
	// some sanity checking
	soa::BoolPtr master = rcp->get<soa::Bool>("master");
	if (!master)
	{
		UT_DEBUGMSG(("Error reading master field\n"));
		return acs::SOAP_ERROR_GENERIC;
	}
	if (!realm_address || realm_address->value().size() == 0 || !realm_port || realm_port->value() <= 0 || !cookie || cookie->value().size() == 0)
	{
		UT_DEBUGMSG(("Invalid realm login information\n"));
		return acs::SOAP_ERROR_GENERIC;
	}
	soa::StringPtr filename_ptr = rcp->get<soa::String>("filename");
	if (!filename_ptr)
	{
		UT_DEBUGMSG(("Error reading filename field\n"));
		return acs::SOAP_ERROR_GENERIC;
	}
	// check the filename; it shouldn't ever be empty, but just check nonetheless
	// TODO: append a number if the filename happens to be empty
	std::string filename = filename_ptr->value().size() > 0 ? filename_ptr->value() : "Untitled";
	
	// open the realm connection!
	UT_DEBUGMSG(("realm_address: %s, realm_port: %lld, cookie: %s\n", realm_address->value().c_str(), realm_port->value(), cookie->value().c_str()));
	ConnectionPtr connection = 
		boost::shared_ptr<RealmConnection>(new RealmConnection(m_ssl_ca_file, realm_address->value(), 
								realm_port->value(), cookie->value(), doc_id, master->value(), session_id,
								boost::bind(&ServiceAccountHandler::_handleRealmPacket, this, _1)));

	// TODO: this connect() call is blocking, so it _could_ take a while; we should
	// display a progress bar in that case
	if (!connection->connect())
	{
		UT_DEBUGMSG(("Error connecting to realm %s:%d\n", realm_address->value().c_str(), realm_port->value()));
		return acs::SOAP_ERROR_GENERIC;
	}
	
	// load the document
	acs::SOAP_ERROR open_result = master->value()
				? _openDocumentMaster(rcp, pDoc, pFrame, session_id, filename)
				: _openDocumentSlave(connection, pDoc, pFrame, filename);

	if (open_result != acs::SOAP_ERROR_OK)
	{
		UT_DEBUGMSG(("Error opening document!\n"));
		// TODO: also nuke the queue!
		connection->disconnect();
		return acs::SOAP_ERROR_GENERIC;
	}
	
	UT_DEBUGMSG(("Document loaded successfully\n"));
	m_connections.push_back(connection);
	return acs::SOAP_ERROR_OK;
}

acs::SOAP_ERROR ServiceAccountHandler::_openDocumentMaster(soa::CollectionPtr rcp, PD_Document** pDoc, XAP_Frame* pFrame, 
																 	const std::string& session_id, const std::string& filename)
{
	UT_return_val_if_fail(rcp || pDoc, acs::SOAP_ERROR_GENERIC);
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, acs::SOAP_ERROR_GENERIC);	
	
	soa::StringPtr document = rcp->get<soa::String>("document");
	UT_return_val_if_fail(document, acs::SOAP_ERROR_GENERIC);
	
	// construct the document
	UT_return_val_if_fail(AbiCollabSessionManager::deserializeDocument(pDoc, document->value(), true) == UT_OK, acs::SOAP_ERROR_GENERIC);
	UT_return_val_if_fail(*pDoc, acs::SOAP_ERROR_GENERIC);

	// set the filename
	gchar* fname = g_strdup(filename.c_str());
	(*pDoc)->setFilename(fname);
	
	// Now register a serviceExporter to handle remote saves
	// via a signal.
	m_pExport = new AbiCollabService_Export(*pDoc,this);
	(*pDoc)->addListener(m_pExport,	&m_iListenerID);
	
	// start the session
	UT_UTF8String sSessionId = session_id.c_str();
	pManager->startSession(*pDoc, sSessionId, pFrame);
	
	return acs::SOAP_ERROR_OK;
}

acs::SOAP_ERROR ServiceAccountHandler::_openDocumentSlave(ConnectionPtr connection, PD_Document** pDoc, XAP_Frame* pFrame, const std::string& filename)
{
	UT_DEBUGMSG(("ServiceAccountHandler::_openDocumentSlave()\n"));
	UT_return_val_if_fail(connection, acs::SOAP_ERROR_GENERIC);
	UT_return_val_if_fail(pDoc, acs::SOAP_ERROR_GENERIC);
	
	// get the progress dialog
	XAP_Frame* pDlgFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pDlgFrame, acs::SOAP_ERROR_GENERIC);

	XAP_DialogFactory* pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	UT_return_val_if_fail(pFactory, acs::SOAP_ERROR_GENERIC);

	AP_Dialog_GenericProgress* pDlg = static_cast<AP_Dialog_GenericProgress*>(
				pFactory->requestDialog(ServiceAccountHandler::getDialogGenericProgressId())
			);		
	pDlg->setTitle("Retrieving Document");
	pDlg->setInformation("Please wait while retrieving document...");

	// setup the information for the callback to use when the document comes in
	connection->loadDocumentStart(pDlg, pDoc, pFrame, filename);
	
	// run the dialog
	pDlg->runModal(pDlgFrame);
	UT_DEBUGMSG(("Progress dialog destroyed...\n"));
	bool m_cancelled = pDlg->getAnswer() == AP_Dialog_GenericProgress::a_CANCEL;
	pFactory->releaseDialog(pDlg);		
	connection->loadDocumentEnd();
	
	if (m_cancelled)
		return acs::SOAP_ERROR_GENERIC;
	// Now register a serviceExporter to handle remote saves
	// via a signal.

	m_pExport = new AbiCollabService_Export(*pDoc,this);
	(*pDoc)->addListener(m_pExport,&m_iListenerID);

	return acs::SOAP_ERROR_OK;
}

// FIXME: this function can be called from another thread; 
// don't allow it to access data from the main thread
// NOTE: saveDocument can be called from a thread other than our mainloop;
// Don't let access or modify any data from the mainloop!
// FIXME FIXME FIXME: we should NOT touch m_connections!!!!
UT_Error ServiceAccountHandler::saveDocument(PD_Document* pDoc, const UT_UTF8String& sSessionId)
{
	UT_DEBUGMSG(("Saving document with session id %s to webservice!\n", sSessionId.utf8_str()));
	UT_return_val_if_fail(pDoc, UT_ERROR);
	
	// find the realm connection beloning to this session to fetch the document id
	for (std::vector<boost::shared_ptr<RealmConnection> >::iterator it = m_connections.begin(); it != m_connections.end(); it++)
	{
		boost::shared_ptr<RealmConnection> connection_ptr = *it;
		UT_continue_if_fail(connection_ptr);
		if (connection_ptr->session_id() == sSessionId.utf8_str())
		{
			UT_DEBUGMSG(("Saving document id %lld\n", connection_ptr->doc_id()));
			
			const std::string uri = getProperty("uri");
			const std::string email = getProperty("email");
			const std::string password = getProperty("password");
			boost::shared_ptr<std::string> document(new std::string(""));
			if (AbiCollabSessionManager::serializeDocument(pDoc, *document, true) != UT_OK)
				return UT_ERROR;
			
			// construct a SOAP method call to gets our documents
			soa::function_call fc("saveDocument", "saveDocumentResponse");
			fc("email", email)
				("password", password)
				("doc_id", connection_ptr->doc_id())
				(soa::Base64Bin("data", document));

			// execute the call and ignore the result (the revision number stored)
			try {
				soa::GenericPtr soap_result = soup_soa::invoke(uri, soa::method_invocation("urn:AbiCollabSOAP", fc), m_ssl_ca_file);
				UT_return_val_if_fail(soap_result, UT_ERROR);
			} catch (soa::SoapFault& fault) {
				UT_DEBUGMSG(("Caught a soap fault: %s (error code: %s)!\n", 
						 fault.detail() ? fault.detail()->value().c_str() : "(null)",
						 fault.string() ? fault.string()->value().c_str() : "(null)"));
				return UT_ERROR;
			}

			return UT_OK;
		}
	}

	return UT_ERROR;
}

void ServiceAccountHandler::signal(const Event& event, const Buddy* pSource)
{
	UT_DEBUGMSG(("ServiceAccountHandler::signal()\n"));
	
	// NOTE: do NOT let AccountHandler::signal() send broadcast packets!
	// It will send them to all buddies, including the ones we created
	// to list the available documents: ServiceBuddies. They are just fake
	// buddies however, and can't receive real packets. Only RealmBuddy's
	// can be sent packets


	// Note: there is no real need to pass the PCT_CloseSessionEvent and
	// PCT_DisjoinSessionEvent signals to the AccountHandler::signal()
	// function: that one will send all buddies the 'session is closed'
	// signal. However, on this backend, the abicollab.net realm will 
	// handle that for us
	
	switch (event.getClassType())
	{
		case PCT_CloseSessionEvent:
			{
				UT_DEBUGMSG(("Got a PCT_CloseSessionEvent, disconnecting the realm connection...\n"));
				const CloseSessionEvent cse = static_cast<const CloseSessionEvent&>(event);
				UT_return_if_fail(!pSource); // we shouldn't receive these events over the wire on this backend
				ConnectionPtr connection_ptr = _getConnection(cse.getSessionId().utf8_str());
				UT_return_if_fail(connection_ptr);
				connection_ptr->disconnect();
			}
			break;
		case PCT_DisjoinSessionEvent:
			{
				UT_DEBUGMSG(("Got a PCT_DisjoinSessionEvent, disconnecting the realm connection...\n"));
				const DisjoinSessionEvent dse = static_cast<const DisjoinSessionEvent&>(event);
				UT_return_if_fail(!pSource); // we shouldn't receive these events over the wire on this backend
				ConnectionPtr connection_ptr = _getConnection(dse.getSessionId().utf8_str());
				UT_return_if_fail(connection_ptr);
				connection_ptr->disconnect();
			}
			break;
		case PCT_StartSessionEvent:
			// TODO: users should get this I guess, but I don't know a proper way to implement this yet
			break;
		default:	
			// TODO: implement me
			break;
	}
}

// NOTE: _listDocuments can be called from a thread other than our mainloop;
// Don't let access or modify any data from the mainloop!
acs::SOAP_ERROR ServiceAccountHandler::_listDocuments(
					const std::string uri, const std::string email, const std::string password,
					SessionBuddyPtr sessions_ptr)
{
	UT_DEBUGMSG(("ServiceAccountHandler::_listDocuments()\n"));
	UT_return_val_if_fail(sessions_ptr, acs::SOAP_ERROR_GENERIC);

	// construct a SOAP method call to gets our documents
	soa::function_call fc("listDocuments", "listDocumentsResponse");
	fc("email", email)("password", password);

	soa::GenericPtr soap_result;
	try {
		soap_result = soup_soa::invoke(uri, soa::method_invocation("urn:AbiCollabSOAP", fc), m_ssl_ca_file);
	} catch (soa::SoapFault& fault) {
		UT_DEBUGMSG(("Caught a soap fault: %s (error code: %s)!\n", 
					 fault.detail() ? fault.detail()->value().c_str() : "(null)",
					 fault.string() ? fault.string()->value().c_str() : "(null)"));
		return acs::error(fault);
	}
	if (!soap_result)
		return acs::SOAP_ERROR_GENERIC;

	// handle the result
	soa::CollectionPtr rcp = soap_result->as<soa::Collection>("return");
	UT_return_val_if_fail(rcp, acs::SOAP_ERROR_GENERIC);
	
	// load our own files
	GetSessionsResponseEvent gsre;
	_parseSessionFiles(rcp->get< soa::Array<soa::GenericPtr> >("files"), gsre);
	sessions_ptr->push_back( std::make_pair(gsre, new ServiceBuddy(this, email.c_str()) ));

	// load the files from our friends
	if (soa::ArrayPtr friends_array = rcp->get< soa::Array<soa::GenericPtr> >("friends"))
		if (abicollab::FriendArrayPtr friends = friends_array->construct<abicollab::Friend>())
			for (size_t i = 0; i < friends->size(); i++)
				if (abicollab::FriendPtr friend_ = friends->operator[](i))
				{
					UT_DEBUGMSG(("Got a friend: %s <%s>\n", friend_->name.c_str(), friend_->email.c_str()));
					if (friend_->email != "")
					{
						// add this friend's documents by generating a GetSessionsResponseEvent
						// to populate all the required document structures
						GetSessionsResponseEvent gsre;
						_parseSessionFiles(friend_->files, gsre);
						sessions_ptr->push_back( std::make_pair(gsre, new ServiceBuddy(this, friend_->email.c_str()) ));
					}				
				}
	
	// load the files from our groups
	if (soa::ArrayPtr groups_array = rcp->get< soa::Array<soa::GenericPtr> >("groups"))
		if (abicollab::GroupArrayPtr groups = groups_array->construct<abicollab::Group>())
			for (size_t i = 0; i < groups->size(); i++)
				if (abicollab::GroupPtr group_ = groups->operator[](i))
				{
					UT_DEBUGMSG(("Got a group: %s\n", group_->name.c_str()));
					if (group_->name != "")
					{
						// add this friend's documents by generating a GetSessionsResponseEvent
						// to populate all the required document structures
						GetSessionsResponseEvent gsre;
						_parseSessionFiles(group_->files, gsre);
						sessions_ptr->push_back( std::make_pair(gsre, new ServiceBuddy(this, group_->name.c_str()) ));
					}				
				}
	
	return acs::SOAP_ERROR_OK;
}


void ServiceAccountHandler::_listDocuments_cb(acs::SOAP_ERROR error, SessionBuddyPtr sessions_ptr)
{
	UT_DEBUGMSG(("ServiceAccountHandler::_listDocuments_cb()\n"));
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);			

	pManager->endAsyncOperation(this);
	
	UT_return_if_fail(sessions_ptr);
	
	switch (error)
	{
		case acs::SOAP_ERROR_OK:
			{
				std::vector<SessionBuddyPair>& sessions = *sessions_ptr;
				for (std::vector<SessionBuddyPair>::iterator it = sessions.begin(); it != sessions.end(); it++)
				{
					ServiceBuddy* pBuddy = (*it).second;
					UT_continue_if_fail(pBuddy);
					Buddy* pExistingBuddy = getBuddy(pBuddy->getName());
					if (!pExistingBuddy)
					{
						pExistingBuddy = pBuddy;
						addBuddy(pBuddy);
					}
					else
						DELETEP(pBuddy);
					_handlePacket(&((*it).first), pExistingBuddy, false);
				}
			}
			break;
		case acs::SOAP_ERROR_INVALID_PASSWORD:
			{
				// FIXME: should we ask for a password in an async callback?
				const std::string email = getProperty("email");
				std::string password;
				if (askPassword(email, password))
				{
					// store the new password
					addProperty("password", password);
					pManager->storeProfile();

					// re-attempt to fetch the documents list
					pManager->beginAsyncOperation(this);
					SessionBuddyPtr new_sessions_ptr(new std::vector<SessionBuddyPair>());
					boost::shared_ptr<AsyncWorker<acs::SOAP_ERROR> > async_list_docs_ptr(
								new AsyncWorker<acs::SOAP_ERROR>(
									boost::bind(&ServiceAccountHandler::_listDocuments, this, 
												getProperty("uri"), getProperty("email"), getProperty("password"), new_sessions_ptr),
									boost::bind(&ServiceAccountHandler::_listDocuments_cb, this, _1, new_sessions_ptr)
								)
							);
					async_list_docs_ptr->start();
				}
			}
			return;
		default:
			// FIXME: maybe determine the exact error
			/// TODO: show a message box?
			UT_DEBUGMSG(("Caught SOAP error: %d\n", error));
			UT_return_if_fail(error == acs::SOAP_ERROR_OK);
	}
}

void ServiceAccountHandler::_handleJoinSessionRequestResponse(
									JoinSessionRequestResponseEvent* jsre, Buddy* pBuddy, 
									XAP_Frame* pFrame, PD_Document** pDoc, const std::string& filename)
{
	UT_return_if_fail(jsre);
	UT_return_if_fail(pBuddy);
	UT_return_if_fail(pDoc);
	
	UT_DEBUGMSG(("_handleJoinSessionRequestResponse() - pFrame: 0x%x\n", pFrame));
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);
	
	UT_return_if_fail(AbiCollabSessionManager::deserializeDocument(pDoc, jsre->m_sZABW, false) == UT_OK);
	UT_return_if_fail(*pDoc);

	gchar* fname = g_strdup(filename.c_str());
	(*pDoc)->setFilename(fname);

	pManager->joinSession(jsre->getSessionId(), *pDoc, jsre->m_sDocumentId, jsre->m_iRev, pBuddy, pFrame);
}

void ServiceAccountHandler::_handleRealmPacket(RealmConnection& connection)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	// make sure we have handled _all_ packets in the queue before checking
	// the disconnected status
	bool disconnected = !connection.isConnected();
	_handleMessages(connection);

	if (disconnected)
	{
		UT_DEBUGMSG(("RealmConnection is not connected anymore (this was a %s connection)!\n", connection.master() ? "master" : "slave"));
		std::vector<RealmBuddyPtr> buddies = connection.getBuddies();
		for (std::vector<RealmBuddyPtr>::iterator it = buddies.begin(); it != buddies.end(); it++)
		{
			RealmBuddyPtr realm_buddy_ptr = *it;
			UT_continue_if_fail(realm_buddy_ptr);
			UT_DEBUGMSG(("Lost connection to buddy with connection id: %d\n", realm_buddy_ptr->realm_connection_id()));
			pManager->removeBuddy(realm_buddy_ptr.get(), false);
		}
		
		// remove the connection from our connection list
		_removeConnection(connection.session_id());
	}

	// check other things here if needed...
}

ConnectionPtr ServiceAccountHandler::_getConnection(const std::string& session_id)
{
	for (std::vector<ConnectionPtr>::iterator it = m_connections.begin(); it != m_connections.end(); it++)
	{
		UT_continue_if_fail(*it);
		if ((*it)->session_id() == session_id)
			return *it;
	}
	return ConnectionPtr();
}

void ServiceAccountHandler::_removeConnection(const std::string& session_id)
{
	UT_DEBUGMSG(("ServiceAccountHandler::_removeConnection()\n"));
	for (std::vector<ConnectionPtr>::iterator it = m_connections.begin(); it != m_connections.end(); it++)
	{
		UT_continue_if_fail(*it);
		ConnectionPtr connection = *it;
		UT_DEBUGMSG(("looking at connection with session id <%s>, closing session <%s>\n", connection->session_id().c_str(), session_id.c_str()));
		if (connection->session_id() == session_id)
		{
			UT_DEBUGMSG(("connection dropped\n"));
			m_connections.erase(it);
			return;
		}
	}
}

void ServiceAccountHandler::_handleMessages(RealmConnection& connection)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	while (connection.queue().peek())
	{
		rpv1::PacketPtr packet = connection.queue().pop();
		UT_continue_if_fail(packet);

		switch (packet->type())
		{
			case rpv1::PACKET_DELIVER:
				{
					UT_DEBUGMSG(("Received a 'Deliver' packet!\n"));
					boost::shared_ptr<rpv1::DeliverPacket> dp =
							boost::static_pointer_cast<rpv1::DeliverPacket>(packet);					
					UT_return_if_fail(dp->getMessage());
				
					boost::shared_ptr<RealmBuddy> buddy_ptr = 
							connection.getBuddy(dp->getConnectionId());
					UT_return_if_fail(buddy_ptr);
				
					Packet* pPacket = _createPacket(*dp->getMessage(), buddy_ptr.get());
					UT_return_if_fail(pPacket);								

					if (pPacket->getClassType() == PCT_JoinSessionRequestResponseEvent)
					{
						UT_DEBUGMSG(("Trapped a JoinSessionRequestResponseEvent!\n"));
						boost::shared_ptr<PendingDocumentProperties> pdp = connection.getPendingDocProps();
						UT_return_if_fail(pdp);

						UT_DEBUGMSG(("Joining received document..."));
						_handleJoinSessionRequestResponse(static_cast<JoinSessionRequestResponseEvent*>(pPacket), buddy_ptr.get(), pdp->pFrame, pdp->pDoc, pdp->filename);
						DELETEP(pPacket);
						UT_return_if_fail(pdp->pDlg);
						pdp->pDlg->close();
						continue;
					}
				
					// let the default handler handle this packet
					// NOTE: this will delete the packet as well (ugly design)
					handleMessage(pPacket, buddy_ptr.get());				
				}
				break;
			case rpv1::PACKET_USERJOINED:
				{
					UT_DEBUGMSG(("Received a 'User Joined' packet!\n"));
					boost::shared_ptr<rpv1::UserJoinedPacket> ujp =
							boost::static_pointer_cast<rpv1::UserJoinedPacket>(packet);
					if (connection.master())
					{
						UT_DEBUGMSG(("We're master, adding buddy to our buddy list\n"));
						UT_return_if_fail(!ujp->isMaster());
						connection.addBuddy(boost::shared_ptr<RealmBuddy>(
								new RealmBuddy(this, static_cast<UT_uint8>(ujp->getConnectionId()), false, connection)));
					}
					else
					{
						if (ujp->isMaster())
						{
							UT_DEBUGMSG(("Received master buddy (id: %d); we're slave, adding it to our buddy list!\n", ujp->getConnectionId()));
							boost::shared_ptr<RealmBuddy> master_buddy(
										new RealmBuddy(this, static_cast<UT_uint8>(ujp->getConnectionId()), true, connection));
							connection.addBuddy(master_buddy);

							UT_DEBUGMSG(("Sending join session request to master!\n"));
							JoinSessionRequestEvent event( connection.session_id().c_str() );
							send(&event, *master_buddy);
						}
						else
							UT_DEBUGMSG(("Received a slave buddy (id: %d); we're slave; dropping it on the floor for now\n", ujp->getConnectionId()));
					}
				}
				break;			
			case rpv1::PACKET_USERLEFT:
				{
					UT_DEBUGMSG(("Received a 'User Left' packet!\n"));
					boost::shared_ptr<rpv1::UserLeftPacket> ulp =
							boost::static_pointer_cast<rpv1::UserLeftPacket>(packet);
					RealmBuddyPtr realm_buddy_ptr = connection.getBuddy(ulp->getConnectionId());
					if (!realm_buddy_ptr)
						return; // we don't store slave buddies at the moment, so this happens when a slave disconnects
					UT_DEBUGMSG(("removing %s buddy with connection id %d from all sessions\n", realm_buddy_ptr->master() ? "master" : "slave", realm_buddy_ptr->realm_connection_id()));
					pManager->removeBuddy(realm_buddy_ptr.get(), false);
					connection.removeBuddy(ulp->getConnectionId());
					// if this was the master, then we can close the realm connection
					if (realm_buddy_ptr->master())
					{
						UT_DEBUGMSG(("The master buddy left; disconnecting the realm connection!\n"));
						connection.disconnect();
						return;
					}
				}
				break;
			default:
				UT_DEBUGMSG(("Dropping unrecognized packet on the floor (type: 0x%x)!\n", packet->type()));
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
		}
	}
}

// NOTE: _parseSessionFiles can be called from a thread other than our mainloop;
// Don't let access or modify any data from the mainloop!
void ServiceAccountHandler::_parseSessionFiles(soa::ArrayPtr files_array, GetSessionsResponseEvent& gsre)
{
	UT_return_if_fail(files_array);
	
	if (abicollab::FileArrayPtr files = files_array->construct<abicollab::File>())
	{
		for (size_t i = 0; i < files->size(); i++)
		{
			if (abicollab::FilePtr file = files->operator[](i))
			{
				UT_DEBUGMSG(("Got a file: %s (%s bytes, id: %s)\n", file->filename.c_str(), file->filesize.c_str(), file->doc_id.c_str()));
				// NOTE: we can safely use the (unique) document id as the session identifier, 
				// as there is basically only one _ever lasting_ session for each 
				// document stored on abicollab.net
				if (file->doc_id != "" && file->access == "readwrite") {
					gsre.m_Sessions[file->doc_id.c_str()] = file->filename.c_str();
				}
			}
		}
	}
}

