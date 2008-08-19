/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiCollab- Code to enable the modification of remote documents.
 * Copyright (C) 2005 by Martin Sevior
 * Copyright (C) 2006,2007 by Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2007 by One Laptop Per Child
 * Copyright (C) 2008 by AbiSource Corporation B.V.
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

#include <string>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "xav_View.h"
#include "xav_Listener.h"
#include "fl_BlockLayout.h"
#include "pd_Document.h"
#include "ie_types.h"
#include "ev_Mouse.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ap_Strings.h"
#include "xap_Prefs.h"
#include "ap_Frame.h"
#include "ut_path.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <backends/xp/AccountHandler.h>
#include <backends/xp/Buddy.h>
#include <backends/xp/SessionEvent.h>

#include <xp/AbiCollab_Export.h>
#include <xp/AbiCollab_Import.h>
#include <xp/AbiCollab.h>
#include <xp/AbiCollabSessionManager.h>
#include <xp/AbiCollab_Packet.h>
#include <xp/AbiCollab_Command.h>
#include <xp/DiskSessionRecorder.h>

ChangeAdjust::ChangeAdjust(const AbstractChangeRecordSessionPacket& packet, PT_DocPosition iRemoteDocPos, const UT_UTF8String& sRemoteDocUUID) 
	: m_pPacket(static_cast<const AbstractChangeRecordSessionPacket*>(packet.clone())),
	m_iLocalPos( m_pPacket->getPos() ),
	m_iRemoteDocPos(iRemoteDocPos),
	m_sRemoteDocUUID(sRemoteDocUUID)
{
}

ChangeAdjust::~ChangeAdjust()
{
	DELETEP(m_pPacket);
}

// Use this constructor to host a collaboration session
AbiCollab::AbiCollab(PD_Document* pDoc, const UT_UTF8String& sSessionId, XAP_Frame* pFrame)
	: EV_MouseListener(),
	m_pDoc(pDoc),
	m_pFrame(pFrame),
	m_Import(this, pDoc),
	m_Export(this, pDoc),
	m_iDocListenerId(0),
	m_bExportMasked(false),
	m_sId(sSessionId),
	m_pController(NULL),
	m_pActivePacket(NULL),
	m_bIsReverting(false),
	m_pRecorder(NULL),
	m_iMouseLID(-1),
	m_bDoingMouseDrag(false),
	m_eTakeoveState(STS_NONE)
{
	// TODO: this can be made a lil' more efficient, as setDocument
	// will create import and export listeners, which is kinda useless
	// when there is no single collaborator yet
	_setDocument(pDoc, pFrame);
	
#ifdef ABICOLLAB_RECORD_ALWAYS
	startRecording( new DiskSessionRecorder( this ) );
#endif
}

// Use this constructor to join a collaboration session
AbiCollab::AbiCollab(const UT_UTF8String& sSessionId,
						PD_Document* pDoc, 
						const UT_UTF8String& docUUID, 
						UT_sint32 iRev, 
						Buddy* pController, 
						XAP_Frame* pFrame)
	: EV_MouseListener(),
	m_pDoc(pDoc),
	m_pFrame(pFrame),
	m_Import(this, pDoc),
	m_Export(this, pDoc),
	m_iDocListenerId(0),
	m_bExportMasked(false),
	m_sId(sSessionId),
	m_pController(pController),
	m_pActivePacket(NULL),
	m_bIsReverting(false),
	m_pRecorder(NULL),
	m_iMouseLID(-1),
	m_bDoingMouseDrag(false),
	m_eTakeoveState(STS_NONE)
{
	// TODO: this can be made a lil' more efficient, as setDocument
	// will create import and export listeners, which is kinda useless
	// when there is no single collaborator yet
	_setDocument(pDoc, pFrame);

	m_Import.setInitialRemoteRev(pController->getName(), iRev);
	m_Export.addFakeImportAdjust(docUUID, iRev);

	// we will manually have to coalesce changerecords, as we will need
	// to be able to revert every individual changerecord for 
	// collision handling if the session controller tells us too
	pDoc->setCoalescingMask(true);

	addCollaborator(pController);
	
#ifdef ABICOLLAB_RECORD_ALWAYS
	startRecording( new DiskSessionRecorder( this ) );
#endif
}

AbiCollab::~AbiCollab(void)
{
	UT_DEBUGMSG(("AbiCollab::~AbiCollab()\n"));
	
	if (m_iMouseLID != -1)
	{
		XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
		if (pFrame)
		{
			// FIXME: we should do this for all frames that display this document!
			EV_Mouse* pMouse = pFrame->getMouse();
			if (pMouse)
				pMouse->unregisterListener(m_iMouseLID);
		}
	}
	
	if (m_iDocListenerId != 0)
		m_pDoc->removeListener(m_iDocListenerId);
	m_iDocListenerId = 0;
	
	
	DELETEP(m_pRecorder);
}

void AbiCollab::removeCollaborator(const Buddy* pCollaborator)
{
	UT_return_if_fail(pCollaborator);

	for (UT_sint32 i = UT_sint32(m_vecCollaborators.size()) - 1; i >= 0; i--)
	{
		Buddy* pBuddy = m_vecCollaborators[i];
		UT_continue_if_fail(pBuddy);
		
		if (pBuddy->getName() == pCollaborator->getName())
			_removeCollaborator(i);
	}
}

void AbiCollab::_removeCollaborator(UT_sint32 index)
{
	UT_DEBUGMSG(("AbiCollab::_removeCollaborator() - index: %d\n", index));
	UT_return_if_fail(index >= 0 && index < UT_sint32(m_vecCollaborators.size()));

	// TODO: signal the removal of the buddy!!!
	// ...
	
	Buddy* pCollaborator = m_vecCollaborators[index];
	UT_return_if_fail(pCollaborator);
	
	// remove this buddy from the import 'seen revision list'
	m_Import.getRemoteRevisions()[pCollaborator->getName().utf8_str()] = 0;
	
	m_vecCollaborators.erase( m_vecCollaborators.begin() + size_t(index) );
}

void AbiCollab::addCollaborator(Buddy* pCollaborator)
{
	UT_DEBUGMSG(("AbiCollab::addCollaborator()\n"));

	// check for duplicates (as long as we assume a collaborator can only be part of a collaboration session once)
	for (UT_uint32 i = 0; i < m_vecCollaborators.size(); i++)
	{
		Buddy* pBuddy = m_vecCollaborators[i];
		if (pBuddy)
		{
			if (pBuddy->getName() == pCollaborator->getName())
			{
				UT_DEBUGMSG(("Attempting to add buddy '%s' twice to a collaboration session!", pCollaborator->getName().utf8_str()));
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				return;
			}
		}
	}	

	m_vecCollaborators.push_back(pCollaborator);
}

void AbiCollab::removeCollaboratorsForAccount(AccountHandler* pHandler)
{
	UT_DEBUGMSG(("AbiCollab::removeCollaboratorsForAccount()\n"));
	UT_return_if_fail(pHandler);
	
	for (UT_sint32 i = UT_sint32(m_vecCollaborators.size())-1; i >= 0; i--)
	{
		Buddy* pBuddy = m_vecCollaborators[i];
		UT_continue_if_fail(pBuddy);
		
		if (pBuddy->getHandler() == pHandler)
			_removeCollaborator(i);
	}
}

void AbiCollab::_setDocument(PD_Document* pDoc, XAP_Frame* pFrame)
{
	UT_DEBUGMSG(("AbiCollab::setDocument()\n"));
	UT_return_if_fail(pDoc);
	UT_return_if_fail(pFrame);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	// assume clean state
	UT_return_if_fail(m_iDocListenerId==0);

	// update the frame
	m_pDoc = pDoc;

	// register ourselves as a mouse listener
	// FIXME: we should do this for all frames that display this document!
	EV_Mouse* pMouse = pFrame->getMouse();
	if (pMouse)
    {
		m_iMouseLID = pMouse->registerListener(this);
    }
	else
    {
		UT_DEBUGMSG(("No current frame!\n"));
    }

	// add the new export listeners
	UT_uint32 lid = 0;
	pDoc->addListener(static_cast<PL_Listener *>(&m_Export), &lid);
	_setDocListenerId(lid);
	UT_DEBUGMSG(("Added document listener %d\n", lid));
}

void AbiCollab::_fillRemoteRev( Packet* pPacket, const Buddy& oBuddy )
{
	UT_return_if_fail(pPacket);
	
	if (pPacket->getClassType() >= _PCT_FirstChangeRecord && pPacket->getClassType() <= _PCT_LastChangeRecord)
	{
		ChangeRecordSessionPacket* pSessionPacket = static_cast<ChangeRecordSessionPacket*>( pPacket );
		pSessionPacket->setRemoteRev( m_Import.getRemoteRevisions()[oBuddy.getName().utf8_str()] );
	}
	else if (pPacket->getClassType() == PCT_GlobSessionPacket)
	{
		GlobSessionPacket* pSessionPacket = static_cast<GlobSessionPacket*>( pPacket );
		const std::vector<SessionPacket*>& globPackets = pSessionPacket->getPackets();
		for (std::vector<SessionPacket*>::const_iterator cit = globPackets.begin(); cit != globPackets.end(); cit++)
		{
			SessionPacket* globPacket = *cit;
			UT_continue_if_fail(globPacket);
			_fillRemoteRev(globPacket, oBuddy);
		}
	}
}

/*!
 *	Send this packet. Note, the specified packet does still belong to the calling class.
 *	So if we want to store it (for masking), we HAVE to clone it first
 */
void AbiCollab::push( Packet* pPacket )
{
	UT_DEBUGMSG(("AbiCollab::push()\n"));
	UT_return_if_fail(pPacket);

	if (m_bIsReverting)
	{
		UT_DEBUGMSG(("This packet was generated by a local revert triggerd in the import; dropping on the floor!\n"));
	}
	else if (m_bExportMasked)
	{
		m_vecMaskedPackets.push_back( pPacket->clone() );
	}
	else
	{
		// record
		if (m_pRecorder)
			m_pRecorder->storeOutgoing( const_cast<const Packet*>( pPacket ) );
		
		// TODO: this could go in the session manager
		UT_DEBUGMSG(("Pusing packet to %d collaborators\n", m_vecCollaborators.size()));
		for (UT_uint32 i = 0; i < m_vecCollaborators.size(); i++)
		{
			Buddy* pCollaborator = m_vecCollaborators[i];
			if (pCollaborator)
			{
				UT_DEBUGMSG(("Pushing packet to collaborator with name: %s\n", pCollaborator->getName().utf8_str()));
				AccountHandler* pHandler = pCollaborator->getHandler();
				if (pHandler)
				{
					// overwrite remote revision for this collaborator
					_fillRemoteRev( pPacket, *pCollaborator );
					
					// send!
					bool res = pHandler->send(pPacket, *pCollaborator);
					if (!res)
                    {
						UT_DEBUGMSG(("Error sending a packet!\n"));
                    }
				}
			}
			else
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}
}

bool AbiCollab::push( Packet* pPacket, const Buddy& collaborator)
{
	UT_return_val_if_fail(pPacket, false);
	AccountHandler* pHandler = collaborator.getHandler();
	UT_return_val_if_fail(pHandler, false);
	
	// record
	if (m_pRecorder)
		m_pRecorder->storeOutgoing( const_cast<const Packet*>( pPacket ), collaborator );

	// overwrite remote revision for this collaborator
	_fillRemoteRev( pPacket, collaborator );
	
	// send!
	bool res = pHandler->send(pPacket, collaborator);
	if (!res)
    {
		UT_DEBUGMSG(("Error sending a packet to collaborator %s!\n", collaborator.getName().utf8_str()));
    }
	return res;
}

void AbiCollab::maskExport()
{
	m_bExportMasked = true;
	m_vecMaskedPackets.clear();
}

const std::vector<Packet*>& AbiCollab::unmaskExport()
{
	m_bExportMasked = false;
	return m_vecMaskedPackets;
}

void AbiCollab::import(SessionPacket* pPacket, const Buddy& collaborator)
{
	UT_DEBUGMSG(("AbiCollab::import()\n"));
	UT_return_if_fail(pPacket);
	
	if (m_bDoingMouseDrag)
	{
		// we block incoming packets while dragging the mouse; this prevents 
		// scary race conditions from occuring, like importing a 'delete image' packet
		// when you are just dragging said image around
		UT_DEBUGMSG(("We are currently dragging something around; deferring packet import until after the release!\n"));
		m_vecIncomingQueue.push_back(
					std::make_pair(	static_cast<SessionPacket*>(pPacket->clone()), collaborator.clone() ));
		return;
	}

	// record the incoming packet
	if (m_pRecorder)
		m_pRecorder->storeIncoming( pPacket, collaborator );

	// execute an alternative packet handling path when this session is being 
	// taken over by another collaborator
	if (m_eTakeoveState != STS_NONE)
	{
		_handleSessionTakeover(pPacket, collaborator);
		return;
	}

	// import the packet; note that it might be denied due to collisions
	maskExport();
	if (AbstractChangeRecordSessionPacket::isInstanceOf(*pPacket))
		m_pActivePacket = static_cast<const AbstractChangeRecordSessionPacket*>(pPacket);
	m_Import.import(*pPacket, collaborator);
	m_pActivePacket = NULL;
	const std::vector<Packet*>& maskedPackets = unmaskExport();
	
	if (isLocallyControlled() && maskedPackets.size() > 0)
	{
		UT_DEBUGMSG(("Forwarding message (%u packets) from %s\n", maskedPackets.size(), collaborator.getName().utf8_str()));
		
		// It seems we are in the center of a collaboration session.
		// It's our duty to reroute the packets to the other collaborators
		for (UT_uint32 i = 0; i < m_vecCollaborators.size(); i++)
		{
			// send all masked packets during import to everyone, except to the
			// person who initialy sent us the packet
			Buddy* pBuddy = m_vecCollaborators[i];
			if (pBuddy && pBuddy->getName() != collaborator.getName())
			{
				UT_DEBUGMSG(("Forwarding message from %s to %s\n", collaborator.getName().utf8_str(), pBuddy->getName().utf8_str()));
				for (std::vector<Packet*>::const_iterator cit=maskedPackets.begin(); cit!=maskedPackets.end(); cit++)
				{
					Packet* maskedPacket = (*cit);
					push( maskedPacket, *pBuddy );
				}
			}
		}
	}
}

void AbiCollab::addChangeAdjust(ChangeAdjust* pAdjust)
{
	UT_return_if_fail(pAdjust);

	if (m_bIsReverting)
	{
		UT_DEBUGMSG(("This changeadjust was generated by a local revert triggerd in the import; dropping on the floor!\n"));
		return;
	}

	m_Export.getAdjusts()->addItem(pAdjust);
}

void AbiCollab::initiateSessionTakeover(const Buddy* pNewMaster)
{
	UT_return_if_fail(pNewMaster);

	SessionTakeoverRequestPacket promoteTakeoverPacket(m_sId, m_pDoc->getOrigDocUUIDString(), true);
	SessionTakeoverRequestPacket normalTakeoverPacket(m_sId, m_pDoc->getOrigDocUUIDString(), false);
	for (std::vector<Buddy*>::iterator it = m_vecCollaborators.begin(); it != m_vecCollaborators.end(); it++)
	{
		Buddy* pBuddy = *it;
		UT_continue_if_fail(pBuddy);

		AccountHandler* pHandler = pBuddy->getHandler();
		UT_continue_if_fail(pHandler);
		
		pHandler->send(pNewMaster == pBuddy ? &promoteTakeoverPacket : &normalTakeoverPacket, *pBuddy);
	}

	m_eTakeoveState = STS_TAKEOVER_REQUEST;
}

void AbiCollab::startRecording( SessionRecorderInterface* pRecorder )
{
	UT_return_if_fail(pRecorder);
	
	const UT_GenericVector<ChangeAdjust *>* pExpAdjusts = m_Export.getAdjusts();
	UT_return_if_fail(pExpAdjusts);
	
	// create initial document packet to recorder
	// so the recorder knows the initial state
	// serialize entire document into string
	JoinSessionRequestResponseEvent jsre( getSessionId() );
	if (AbiCollabSessionManager::serializeDocument(m_pDoc, jsre.m_sZABW, false /* no base64 */) == UT_OK)
	{
		// set more document properties
		if (!isLocallyControlled())
		{
			UT_ASSERT_HARMLESS(pExpAdjusts->getItemCount() > 0);
			jsre.m_iRev = (pExpAdjusts->getItemCount() > 0 ? pExpAdjusts->getNthItem(pExpAdjusts->getItemCount()-1)->getLocalRev() : 0);
		}
		else
			jsre.m_iRev = m_pDoc->getCRNumber();
		jsre.m_sDocumentId = m_pDoc->getDocUUIDString();
		if (m_pDoc->getFilename())
			jsre.m_sDocumentName = UT_go_basename_from_uri(m_pDoc->getFilename());
				
		// store pointer
		m_pRecorder = pRecorder;
		m_pRecorder->storeOutgoing( &jsre );
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
}

void AbiCollab::stopRecording()
{
	DELETEP(m_pRecorder);
}

void AbiCollab::signalMouse(EV_EditBits eb, UT_sint32 /*xPos*/, UT_sint32 /*yPos*/)
{
	switch (eb & EV_EMO__MASK__)
	{
		case EV_EMO_DRAG:
			// check if we have at least one mouse button down, otherwise this is just a move:
			// AbiWord's event framework is a bit weird, in that it qualifies a move without
			// any buttons as a mouse drag as well
			switch (eb & EV_EMB__MASK__) 
			{
				case EV_EMB_BUTTON0: // no buttons down, not a real drag
					break;
				default:
					UT_DEBUGMSG(("AbiCollab: Mouse drag!\n"));
					m_bDoingMouseDrag = true;
					break;
			}
			break;
		case EV_EMO_DOUBLEDRAG:
			UT_DEBUGMSG(("AbiCollab: Mouse doubledrag!\n"));
			m_bDoingMouseDrag = true;
			break;
		case EV_EMO_RELEASE:
			UT_DEBUGMSG(("AbiCollab: Mouse drag release\n"));
			_releaseMouseDrag();
			break;
		case EV_EMO_DOUBLERELEASE:
			UT_DEBUGMSG(("AbiCollab: Mouse doubledrag release\n"));
			_releaseMouseDrag();
			break;
	}
}

void AbiCollab::_releaseMouseDrag()
{
	m_bDoingMouseDrag = false;
	for (std::vector<std::pair<SessionPacket*,Buddy*> >::iterator it = m_vecIncomingQueue.begin(); it !=  m_vecIncomingQueue.end(); it++)
	{
		std::pair<SessionPacket*,Buddy*>& pair = *it;
		UT_continue_if_fail(pair.first && pair.second);
		
		if (pair.first && pair.second)
			import(pair.first, *pair.second);
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

		DELETEP(pair.first);
		DELETEP(pair.second);
	}
		 
	m_vecIncomingQueue.clear();
}

void AbiCollab::_handleSessionTakeover(SessionPacket* pPacket, const Buddy& collaborator)
{
	UT_DEBUGMSG(("AbiCollab::_handleSessionTakeover()\n"));
	UT_return_if_fail(pPacket);

	// TODO: implement me
}

