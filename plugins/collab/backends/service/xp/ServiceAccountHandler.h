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

#ifndef __SERVICEACCOUNTHANDLER__
#define __SERVICEACCOUNTHANDLER__

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include "xap_Types.h"
#include "ut_string_class.h"
#include <backends/xp/AccountHandler.h>
#include "soa.h"
#include "AbiCollabSaveInterceptor.h"
#include "RealmBuddy.h"
#include "RealmConnection.h"
#include "RealmProtocol.h"
#include "AsioRealmProtocol.h"
#include "ServiceErrorCodes.h"
#include "pl_Listener.h"

namespace acs = abicollab::service;
namespace rpv1 = realm::protocolv1;

class PD_Document;
class GetSessionsResponseEvent;
class JoinSessionRequestResponseEvent;
class ServiceBuddy;
class AbiCollabService_Export;

extern AccountHandlerConstructor ServiceAccountHandlerConstructor;

typedef std::pair<GetSessionsResponseEvent, ServiceBuddy*> SessionBuddyPair;
typedef boost::shared_ptr<std::vector<SessionBuddyPair> > SessionBuddyPtr;

#define SERVICE_ACCOUNT_HANDLER_TYPE "com.abisource.abiword.abicollab.backend.service"

class ServiceAccountHandler : public AccountHandler
{
public:
	ServiceAccountHandler();
	virtual ~ServiceAccountHandler();

	static bool								askPassword(const std::string& email, std::string& password);

	// housekeeping
	virtual UT_UTF8String					getDescription();
	virtual UT_UTF8String					getDisplayType();
	virtual UT_UTF8String					getStorageType();

	// dialog management 
	virtual void							storeProperties();
	static XAP_Dialog_Id					getDialogGenericInputId();
	static XAP_Dialog_Id					getDialogGenericProgressId();

	// connection management
	virtual ConnectResult					connect();
	virtual bool							disconnect();
	virtual bool							isOnline();

	// user management
	virtual Buddy*							constructBuddy(const PropertyMap& props);
	virtual bool							allowsManualBuddies()
		{ return false; }
	
	// packet management
	virtual bool							send(const Packet* packet);
	virtual bool							send(const Packet* packet, const Buddy& buddy);

	// session management
	virtual void							getSessionsAsync();
	virtual void							getSessionsAsync(const Buddy& buddy);
	virtual bool							hasSession(const UT_UTF8String& sSessionId);
	virtual void							joinSessionAsync(const Buddy& buddy, DocHandle& docHandle);
	acs::SOAP_ERROR							openDocument(UT_sint64 doc_id, UT_sint64 revision, const std::string& session_id, PD_Document** pDoc, XAP_Frame* pFrame);
	UT_Error								saveDocument(PD_Document* pDoc, const UT_UTF8String& sSessionId);
	void                                                            removeExporter(void); 
	// signal management
	virtual void							signal(const Event& event, const Buddy* pSource);

	static XAP_Dialog_Id		 			m_iDialogGenericInput;
	static XAP_Dialog_Id		 			m_iDialogGenericProgress;
	static AbiCollabSaveInterceptor			m_saveInterceptor;

private:
	template <class T>
	void _send(boost::shared_ptr<T> packet, boost::shared_ptr<const RealmBuddy> recipient)
	{
		realm::protocolv1::send(*packet, recipient->connection().socket(),
			boost::bind(&ServiceAccountHandler::_write_handler, this,
							asio::placeholders::error, asio::placeholders::bytes_transferred, recipient,
							boost::static_pointer_cast<realm::protocolv1::Packet>(packet)));
	}

	void									_write_handler(const asio::error_code& e, std::size_t bytes_transferred,
													boost::shared_ptr<const RealmBuddy> recipient, boost::shared_ptr<realm::protocolv1::Packet> packet);

	acs::SOAP_ERROR							_listDocuments(const std::string uri, const std::string email, const std::string password, 
													SessionBuddyPtr sessions_ptr);
	void									_listDocuments_cb(acs::SOAP_ERROR error, SessionBuddyPtr sessions_ptr);
	
	acs::SOAP_ERROR							_openDocumentMaster(soa::CollectionPtr rcp, PD_Document** pDoc, XAP_Frame* pFrame, 
													const std::string& session_id, const std::string& filename);
	acs::SOAP_ERROR							_openDocumentSlave(ConnectionPtr connection, PD_Document** pDoc, XAP_Frame* pFrame, 
													const std::string& filename);
	
	void									_handleJoinSessionRequestResponse(
													JoinSessionRequestResponseEvent* jsre, Buddy* pBuddy, 
													XAP_Frame* pFrame, PD_Document** pDoc, const std::string& filename);
	void									_handleRealmPacket(RealmConnection& connection);
	ConnectionPtr							_getConnection(const std::string& session_id);
	void									_removeConnection(const std::string& session_id);
	void									_handleMessages(RealmConnection& connection);
	void									_parseSessionFiles(soa::ArrayPtr files_array, GetSessionsResponseEvent& gsre);
	virtual	void							_handlePacket(Packet* packet, Buddy* buddy, bool autoAddBuddyOnJoin)
		{ AccountHandler::_handlePacket(packet, buddy, false); }


	bool									m_bOnline;  // only used to determine if we are allowed to 
														// communicate with abicollab.net or not
	std::vector<ConnectionPtr>				m_connections;
	std::string								m_ssl_ca_file;
	PL_ListenerId             m_iListenerID;
	AbiCollabService_Export * m_pExport;
	  
};

#endif /* __SERVICEACCOUNTHANDLER__ */
