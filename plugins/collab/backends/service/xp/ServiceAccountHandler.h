/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2008,2009 AbiSource Corporation B.V.
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

#ifndef __SERVICEACCOUNTHANDLER__
#define __SERVICEACCOUNTHANDLER__

#include <string>
#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "xap_Types.h"
#include "ut_string_class.h"

#include "soa.h"
#include <core/account/xp/AccountHandler.h>
#include "AbiCollabSaveInterceptor.h"
#include "AsioRealmProtocol.h"
#include "pl_Listener.h"
#include "RealmConnection.h"
#include "RealmBuddy.h"
#include "RealmProtocol.h"
#include "ServiceBuddy.h"
#include "ServiceErrorCodes.h"

namespace acs = abicollab::service;
namespace rpv1 = realm::protocolv1;

class PD_Document;
class GetSessionsResponseEvent;
class JoinSessionRequestResponseEvent;
class ServiceBuddy;
class AbiCollabService_Export;
class RealmBuddy;

struct DocumentPermissions
{
	std::vector<UT_uint64> read_write;
	std::vector<UT_uint64> read_only;
	std::vector<UT_uint64> group_read_write;
	std::vector<UT_uint64> group_read_only;
	std::vector<UT_uint64> group_read_owner;
};

extern AccountHandlerConstructor ServiceAccountHandlerConstructor;

#define SERVICE_ACCOUNT_HANDLER_TYPE "com.abisource.abiword.abicollab.backend.service"
#define SERVICE_REGISTRATION_URL "https://abicollab.net/user/register"

class ServiceAccountHandler : public AccountHandler
{
public:
	ServiceAccountHandler();
	virtual ~ServiceAccountHandler();

	static bool								askPassword(const std::string& email, std::string& password);
	static bool								askFilename(std::string& filename, bool firsttime);
	static void								ensureExt(std::string& filename, const std::string& extension);

	// housekeeping
	static UT_UTF8String					getStaticStorageType();
	virtual UT_UTF8String					getStorageType()
		{ return getStaticStorageType(); }
	virtual UT_UTF8String					getDescription();
	virtual UT_UTF8String					getDisplayType();

	// dialog management
	virtual UT_UTF8String					getShareHint(PD_Document* pDoc);
	static XAP_Dialog_Id					getDialogGenericInputId();
	static XAP_Dialog_Id					getDialogGenericProgressId();

	// connection management
	virtual ConnectResult					connect();
	virtual bool							disconnect();
	virtual bool							isOnline();
	ConnectionPtr							getConnection(PD_Document* pDoc);

	// user management
	virtual void							getBuddiesAsync();
	virtual BuddyPtr						constructBuddy(const PropertyMap& props);
	virtual BuddyPtr						constructBuddy(const std::string& descriptor, BuddyPtr pBuddy);
	virtual bool							allowsManualBuddies()
		{ return false; }
	virtual bool							recognizeBuddyIdentifier(const std::string& identifier);
	virtual void							forceDisconnectBuddy(BuddyPtr pBuddy);
	virtual bool							hasAccess(const std::vector<std::string>& vAcl, BuddyPtr pBuddy);
	virtual bool							hasPersistentAccessControl()
		{ return true; }
	virtual bool							canShare(BuddyPtr pBuddy);

	// packet management
	virtual bool							send(const Packet* packet);
	virtual bool							send(const Packet* packet, BuddyPtr pBuddy);

	// session management
	virtual void							getSessionsAsync();
	virtual void							getSessionsAsync(const Buddy& buddy);
	virtual bool							startSession(PD_Document* pDoc, const std::vector<std::string>& vAcl, AbiCollab** pSession);
	virtual bool							getAcl(AbiCollab* pSession, std::vector<std::string>& vAcl);
	virtual bool							setAcl(AbiCollab* pSession, const std::vector<std::string>& vAcl);
	virtual void							joinSessionAsync(BuddyPtr pBuddy, DocHandle& docHandle);
	virtual bool							hasSession(const std::string& sSessionId);
	acs::SOAP_ERROR							openDocument(UT_uint64 doc_id, UT_uint64 revision, const std::string& session_id, PD_Document** pDoc, XAP_Frame* pFrame);
	soa::function_call_ptr					constructListDocumentsCall();
	soa::function_call_ptr					constructSaveDocumentCall(PD_Document* pDoc, ConnectionPtr connection_ptr);
	void									removeExporter(void);
	virtual bool							allowsSessionTakeover()
		{ return true; }

	// signal management
	virtual void							signal(const Event& event, BuddyPtr pSource);

	// misc functions
	const std::string&						getCA() const
		{ return m_ssl_ca_file; }
	static bool							parseUserInfo(const std::string& userinfo, uint64_t& user_id);

	static XAP_Dialog_Id		 			m_iDialogGenericInput;
	static XAP_Dialog_Id		 			m_iDialogGenericProgress;
	static AbiCollabSaveInterceptor			m_saveInterceptor;

private:
	ConnectionPtr							_realmConnect(soa::CollectionPtr rcp,
													UT_uint64 doc_id, const std::string& session_id, bool master);

	ServiceBuddyPtr							_getBuddy(const UT_UTF8String& descriptor);
	ServiceBuddyPtr							_getBuddy(ServiceBuddyPtr pBuddy);
	ServiceBuddyPtr							_getBuddy(ServiceBuddyType type, uint64_t user_id);


	template <class T>
	void _send(boost::shared_ptr<T> packet, RealmBuddyPtr recipient)
	{
		realm::protocolv1::send(*packet, recipient->connection()->socket(),
			boost::bind(&ServiceAccountHandler::_write_handler, this,
							asio::placeholders::error, asio::placeholders::bytes_transferred, recipient,
							boost::static_pointer_cast<rpv1::Packet>(packet)));
	}

	void									_write_handler(const asio::error_code& e, std::size_t bytes_transferred,
													boost::shared_ptr<const RealmBuddy> recipient, boost::shared_ptr<rpv1::Packet> packet);

	void									_write_result(const asio::error_code& e, std::size_t bytes_transferred,
													ConnectionPtr connection, boost::shared_ptr<rpv1::Packet> packet);

	bool									_listDocuments(soa::function_call_ptr fc_ptr,
													const std::string uri, bool verify_webapp_host,
													boost::shared_ptr<std::string> result_ptr);
	void									_listDocuments_cb(bool success, soa::function_call_ptr fc_ptr, boost::shared_ptr<std::string> result_ptr);

	acs::SOAP_ERROR							_openDocumentMaster(ConnectionPtr connection, soa::CollectionPtr rcp, PD_Document** pDoc, XAP_Frame* pFrame,
													const std::string& session_id, const std::string& filename, bool bLocallyOwned);
	acs::SOAP_ERROR							_openDocumentSlave(ConnectionPtr connection, PD_Document** pDoc, XAP_Frame* pFrame,
													const std::string& filename, bool bLocallyOwned);
	bool									_getConnections();
	bool									_getPermissions(uint64_t doc_id, DocumentPermissions& perms);
	bool									_setPermissions(UT_uint64 doc_id, DocumentPermissions& perms);

	void									_handleJoinSessionRequestResponse(
													JoinSessionRequestResponseEvent* jsre, BuddyPtr pBuddy,
													XAP_Frame* pFrame, PD_Document** pDoc, const std::string& filename,
													bool bLocallyOwned);
	void									_handleRealmPacket(ConnectionPtr connection);
	ConnectionPtr							_getConnection(const std::string& session_id);
	void									_removeConnection(const std::string& session_id);
	void									_handleMessages(ConnectionPtr connection);
	void									_parseSessionFiles(soa::ArrayPtr files_array, GetSessionsResponseEvent& gsre);
	bool									_splitDescriptor(const std::string& descriptor, uint64_t& user_id, uint8_t& conn_id, std::string& domain);
	std::string								_getDomain(const std::string& protocol);
	std::string								_getDomain();
	bool									_parseUserInfo(const std::string& userinfo, uint64_t& user_id);

	bool									m_bOnline;  // only used to determine if we are allowed to
														// communicate with abicollab.net or not
	std::vector<ConnectionPtr>				m_connections;
	std::map<uint64_t, DocumentPermissions> m_permissions;
	std::string								m_ssl_ca_file;
	PL_ListenerId             m_iListenerID;
	AbiCollabService_Export * m_pExport;

};

#endif /* __SERVICEACCOUNTHANDLER__ */
