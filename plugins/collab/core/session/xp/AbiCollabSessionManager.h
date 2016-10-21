/*
 * Copyright (C) 2005 by Martin Sevior
 * Copyright (C) 2006 by Marc Maurer <uwog@uwog.net>
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

#ifndef ABICOLLABSESSIONMANAGER_H
#define ABICOLLABSESSIONMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "ut_types.h"
#include "pt_Types.h"
#include "stdio.h"
#include "ut_string_class.h"
#include "xap_Dialog_Id.h"
#include "xap_Types.h"
#include "ie_imp.h"
#include <account/xp/AccountHandler.h>
#include <libxml/tree.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class AbiCollab;
class Event;
class EventListener;

class AbiCollabSessionManager
{
public:
	static AbiCollabSessionManager*				getManager();

	virtual		       							~AbiCollabSessionManager(void);

	// helper functions
	static UT_Error								serializeDocument(const PD_Document* pDoc, std::string& document, bool encodeBase64=true );
	static UT_Error								deserializeDocument(PD_Document** pDoc, const std::string& document, bool isEncodedBase64=true);

	// dialog code
	bool										registerDialogs(void);
	bool										unregisterDialogs(void);
	XAP_Dialog_Id								getDialogShareId()
		{ return m_iDialogShare; }
	XAP_Dialog_Id								getDialogJoinId()
		{ return m_iDialogJoin; }
	XAP_Dialog_Id								getDialogAccountsId()
		{ return m_iDialogAccounts; }
	XAP_Dialog_Id								getDialogAddAccountId()
		{ return m_iDialogAddAccount; }
	XAP_Dialog_Id								getDialogEditAccountId()
		{ return m_iDialogEditAccount; }
	XAP_Dialog_Id								getDialogAddBuddyId()
		{ return m_iDialogAddBuddy; }

	#ifdef _WIN32
	// On Windows, we must share our HMODULE/HINSTANCE so we can do gui
	void										setInstance(HINSTANCE hModule)
		{ m_hModule = hModule; }
	HINSTANCE									getInstance()
		{ return m_hModule; }

	#endif

	// profile code
	void										loadProfile();
	void										storeProfile();

	// session code
	bool										destroySession(PD_Document* pDoc);
	bool										destroySession(AbiCollab* pSession);
	void										disconnectSession(AbiCollab* pSession);
	void										disconnectSessions();
	AbiCollab*									getSession(PD_Document* pDoc);
	AbiCollab*									getSessionFromDocumentId(const UT_UTF8String& sDocumentId);
	AbiCollab*									getSessionFromSessionId(const UT_UTF8String& sSessionId);
	const UT_GenericVector<AbiCollab *>&		getSessions(void) const
		{ return m_vecSessions;}
	AbiCollab*									startSession(PD_Document* pDoc, UT_UTF8String& sNewSessionId, AccountHandler* pAclAccount,
														bool bLocallyOwned,	XAP_Frame* pFrame, const UT_UTF8String& masterDescriptor);
	void										closeSession(AbiCollab* pSession, bool canConfirm);
	void										closeSessions();
	void										joinSessionInitiate(BuddyPtr pBuddy, DocHandle* pDocHandle);
	void										joinSession(const UT_UTF8String& sSessionId, PD_Document* pDoc,
														const UT_UTF8String& docUUID, UT_sint32 iRev, UT_sint32 iAuthorId,
														BuddyPtr pCollaborator, AccountHandler* pAclAccount, bool bLocallyOwned,
														XAP_Frame *pFrame);
	void										joinSession(AbiCollab* pSession, BuddyPtr pCollaborator);
	void										disjoinSession(const UT_UTF8String& sSessionId);
	bool										isLocallyControlled(PD_Document* pDoc);
	bool										isInSession(PD_Document* pDoc);
	bool										isActive(const UT_UTF8String& sSessionId);
	void										removeBuddy(BuddyPtr pBuddy, bool graceful = true);
	void										updateAcl(AbiCollab* pSession, AccountHandler* pAccount, const std::vector<std::string> acl);
	XAP_Frame*									findFrameForSession(AbiCollab* pSession);

	// account code
	bool										registerAccountHandlers(void);
	bool										unregisterAccountHandlers(void);
	const std::map<UT_UTF8String, AccountHandlerConstructor>&
												getRegisteredAccountHandlers(void)
		{ return m_regAccountHandlers; }
	bool										addAccount(AccountHandler* pHandler);
	const std::vector<AccountHandler *>&		getAccounts() const
		{ return m_vecAccounts; }
	void										destroyAccounts();
	bool										destroyAccount(AccountHandler* pHandler);
	void										setDocumentHandles(BuddyPtr buddy, const UT_GenericVector<DocHandle*>& vDocHandle);
	BuddyPtr									constructBuddy(const std::string& identifier, BuddyPtr pBuddy);

	// packet handling
	bool										processPacket(AccountHandler& handler, Packet* pPacket, BuddyPtr buddy);

	// signalling code
	void										registerEventListener(EventListener* pListener);
	void										unregisterEventListener(EventListener* pListener);
	void										signal(const Event& event, BuddyPtr pSource = BuddyPtr());

	// asynchronous operation handling
	void										beginAsyncOperation(AbiCollab* pSession);
	void										endAsyncOperation(AbiCollab* pSession);
	void										beginAsyncOperation(AccountHandler* pSession);
	void										endAsyncOperation(AccountHandler* pSession);

	// file format handling
	void										unregisterSniffers(void);

	AbiCollabSessionManager(void); // TODO: this constructor shouldn't be public

private:
	bool										_setupFrame(XAP_Frame** pFrame, PD_Document* pDoc);

	// asynchronous operation handling
	void										_deleteSession(AbiCollab* pSession);
	void										_deleteAccount(AccountHandler* pHandler);
	bool										_nullUpdate();

	// session code
	bool										_canInitiateSessionTakeover(AbiCollab* pSession);

	static AbiCollabSessionManager* 			m_pManager;

	// dialog code
	XAP_Dialog_Id								m_iDialogShare;
	XAP_Dialog_Id								m_iDialogJoin;
	XAP_Dialog_Id								m_iDialogAccounts;
	XAP_Dialog_Id								m_iDialogAddAccount;
	XAP_Dialog_Id								m_iDialogEditAccount;
	XAP_Dialog_Id								m_iDialogAddBuddy;

	#ifdef _WIN32
	HINSTANCE 									m_hModule;
	#endif

	// session code
	UT_GenericVector<AbiCollab *>				m_vecSessions;

	// account code
	std::map<UT_UTF8String, AccountHandlerConstructor>	m_regAccountHandlers;
	std::vector<AccountHandler *>				m_vecAccounts;
	UT_GenericVector<EventListener *>			m_vecEventListeners;

	// asynchronous opertation registration
	std::map<AbiCollab*, int>					m_asyncSessionOps;
	std::map<AccountHandler*, int>				m_asyncAccountOps;

	// file format sniffers
	std::vector<IE_ImpSniffer*>					m_vImpSniffers;
};

#endif /* ABICOLLABSESSIONMANAGER_H */
