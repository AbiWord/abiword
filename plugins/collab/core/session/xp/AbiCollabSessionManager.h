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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef ABICOLLABSESSIONMANAGER_H
#define ABICOLLABSESSIONMANAGER_H

#include <string>
#include <vector>
#include "ut_types.h"
#include "pt_Types.h"
#include "stdio.h"
#include "ut_string_class.h"
#include "xap_Dialog_Id.h"
#include "xap_Types.h"
#include <backends/xp/AccountHandler.h>
#include <libxml/tree.h>

#ifdef WIN32
#include <windows.h>
#endif

class AbiCollab;
class RawPacket;

class Event;
class EventListener;
	
class ABI_EXPORT AbiCollabSessionManager
{
public:
	static AbiCollabSessionManager*				getManager();
	
	virtual		       							~AbiCollabSessionManager(void);

	// helper functions
	static UT_Error								serializeDocument(const PD_Document* pDoc, std::string& document, bool encodeBase64=true );
	static UT_Error								deserializeDocument(PD_Document** pDoc, std::string& document, bool isEncodedBase64=true);	

	// dialog code
	bool										registerDialogs(void);
	bool										unregisterDialogs(void);
	XAP_Dialog_Id								getDialogJoinId()
		{ return m_iDialogJoin; }
	XAP_Dialog_Id								getDialogAccountsId()
		{ return m_iDialogAccounts; }
	XAP_Dialog_Id								getDialogAddAccountId()
		{ return m_iDialogAddAccount; }
	XAP_Dialog_Id								getDialogAddBuddyId()
		{ return m_iDialogAddBuddy; }

	#ifdef WIN32
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
	AbiCollab*									startSession(PD_Document* pDoc, UT_UTF8String& sNewSessionId);
	void										closeSession(AbiCollab* pSession, bool canConfirm);
	void										closeSessions();
	void										joinSessionInitiate(Buddy* pBuddy, DocHandle* pDocHandle);
	void										joinSession(const UT_UTF8String& sSessionId, PD_Document* pDoc, const UT_UTF8String& docUUID, UT_sint32 iRev, Buddy* pCollaborator);
	void										joinSession(AbiCollab* pSession, Buddy* pCollaborator);
	void										disjoinSession(const UT_UTF8String& sSessionId);	
	bool										isLocallyControlled(PD_Document* pDoc);
	bool										isInSession(PD_Document* pDoc);
	bool										isActive(const UT_UTF8String& sSessionId);
	  
	// account code
	bool										registerAccountHandlers(void);
	bool										unregisterAccountHandlers(void);
	const UT_GenericVector<AccountHandlerConstructor>&
												getRegisteredAccountHandlers(void)
		{ return m_regAccountHandlers; }
	bool										addAccount(AccountHandler* pHandler);
	const UT_GenericVector<AccountHandler *>&	getAccounts() const
		{ return m_vecAccounts; }
	void										destroyAccounts();
	void										destroyAccount(AccountHandler* pHandler);
	void										setDocumentHandles(Buddy& buddy, const UT_GenericVector<DocHandle*>& vDocHandle);

	// packet handling
	bool										processPacket(AccountHandler& handler, Packet* pPacket, const UT_UTF8String& sBuddyName);
	
	// signalling code
	void										registerEventListener(EventListener* pListener);
	void										unregisterEventListener(EventListener* pListener);
	void										signal(const Event& event, const Buddy* pSource = 0);

		
	AbiCollabSessionManager(void); // TODO: this constructor shouldn't be public

private:
	static AbiCollabSessionManager* 			m_pManager;
	
	// dialog code
	XAP_Dialog_Id								m_iDialogJoin;
	XAP_Dialog_Id								m_iDialogAccounts;
	XAP_Dialog_Id								m_iDialogAddAccount;
	XAP_Dialog_Id								m_iDialogAddBuddy;
	
	#ifdef WIN32
	HINSTANCE 									m_hModule;
	#endif

	// session code	
	UT_GenericVector<AbiCollab *>				m_vecSessions;
	
	// account code
	UT_GenericVector<AccountHandlerConstructor>	m_regAccountHandlers;
	UT_GenericVector<AccountHandler *>			m_vecAccounts;
	UT_GenericVector<EventListener *>			m_vecEventListeners;
};

#endif /* ABICOLLABSESSIONMANAGER_H */
