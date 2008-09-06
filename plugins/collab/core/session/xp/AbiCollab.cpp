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
	m_pController(BuddyPtr()),
	m_pActivePacket(NULL),
	m_bIsReverting(false),
	m_pRecorder(NULL),
	m_iMouseLID(-1),
	m_bDoingMouseDrag(false),
	m_eTakeoveState(STS_NONE),
	m_bProposedController(false),
	m_pProposedController(BuddyPtr())
{
	// TODO: this can be made a lil' more efficient, as setDocument
	// will create import and export listeners, which is kinda useless
	// when there is no single collaborator yet
	_setDocument(pDoc, pFrame);
	
	m_Import.masterInit();
	m_Export.masterInit();

#ifdef ABICOLLAB_RECORD_ALWAYS
	startRecording( new DiskSessionRecorder( this ) );
#endif
}

// Use this constructor to join a collaboration session
AbiCollab::AbiCollab(const UT_UTF8String& sSessionId,
						PD_Document* pDoc, 
						const UT_UTF8String& docUUID, 
						UT_sint32 iRev, 
						BuddyPtr pController, 
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
	m_eTakeoveState(STS_NONE),
	m_bProposedController(false),
	m_pProposedController(BuddyPtr())
{
	// TODO: this can be made a lil' more efficient, as setDocument
	// will create import and export listeners, which is kinda useless
	// when there is no single collaborator yet
	_setDocument(pDoc, pFrame);

	m_Import.slaveInit(pController, iRev);
	m_Export.slaveInit(docUUID, iRev);

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

void AbiCollab::removeCollaborator(BuddyPtr pCollaborator)
{
	UT_return_if_fail(pCollaborator);

	for (UT_sint32 i = UT_sint32(m_vCollaborators.size()) - 1; i >= 0; i--)
	{
		BuddyPtr pBuddy = m_vCollaborators[i];
		UT_continue_if_fail(pBuddy);
		if (pBuddy == pCollaborator)
			_removeCollaborator(i);
	}
}

void AbiCollab::_removeCollaborator(UT_sint32 index)
{
	UT_DEBUGMSG(("AbiCollab::_removeCollaborator() - index: %d\n", index));
	UT_return_if_fail(index >= 0 && index < UT_sint32(m_vCollaborators.size()));

	// TODO: signal the removal of the buddy!!!
	// ...
	
	BuddyPtr pCollaborator = m_vCollaborators[index];
	UT_return_if_fail(pCollaborator);
	
	// remove this buddy from the import 'seen revision list'
	m_Import.getRemoteRevisions()[pCollaborator] = 0;
	
	m_vCollaborators.erase(m_vCollaborators.begin() + size_t(index) );
}

void AbiCollab::addCollaborator(BuddyPtr pCollaborator)
{
	UT_DEBUGMSG(("AbiCollab::addCollaborator()\n"));
	UT_return_if_fail(pCollaborator)

	// check for duplicates (as long as we assume a collaborator can only be part of a collaboration session once)
	for (UT_uint32 i = 0; i < m_vCollaborators.size(); i++)
	{
		BuddyPtr pBuddy = m_vCollaborators[i];
		UT_continue_if_fail(pBuddy);
		if (pBuddy == pCollaborator)
		{
			UT_DEBUGMSG(("Attempting to add buddy '%s' twice to a collaboration session!", pCollaborator->getDescription().utf8_str()));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return;
		}
	}	

	m_vCollaborators.push_back(pCollaborator);
}

void AbiCollab::removeCollaboratorsForAccount(AccountHandler* pHandler)
{
	UT_DEBUGMSG(("AbiCollab::removeCollaboratorsForAccount()\n"));
	UT_return_if_fail(pHandler);
	
	for (UT_sint32 i = UT_sint32(m_vCollaborators.size())-1; i >= 0; i--)
	{
		BuddyPtr pBuddy = m_vCollaborators[i];
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

void AbiCollab::_fillRemoteRev(Packet* pPacket, BuddyPtr pBuddy)
{
	UT_return_if_fail(pPacket);
	UT_return_if_fail(pBuddy);
	
	if (pPacket->getClassType() >= _PCT_FirstChangeRecord && pPacket->getClassType() <= _PCT_LastChangeRecord)
	{
		ChangeRecordSessionPacket* pSessionPacket = static_cast<ChangeRecordSessionPacket*>(pPacket);
		pSessionPacket->setRemoteRev(m_Import.getRemoteRevisions()[pBuddy]);
	}
	else if (pPacket->getClassType() == PCT_GlobSessionPacket)
	{
		GlobSessionPacket* pSessionPacket = static_cast<GlobSessionPacket*>(pPacket);
		const std::vector<SessionPacket*>& globPackets = pSessionPacket->getPackets();
		for (std::vector<SessionPacket*>::const_iterator cit = globPackets.begin(); cit != globPackets.end(); cit++)
		{
			SessionPacket* globPacket = *cit;
			UT_continue_if_fail(globPacket);
			_fillRemoteRev(globPacket, pBuddy);
		}
	}
}

/*!
 *	Send this packet. Note, the specified packet does still belong to the calling class.
 *	So if we want to store it (for masking), we HAVE to clone it first
 */
void AbiCollab::push(Packet* pPacket)
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
		UT_DEBUGMSG(("Pusing packet to %d collaborators\n", m_vCollaborators.size()));
		for (UT_uint32 i = 0; i < m_vCollaborators.size(); i++)
		{
			BuddyPtr pCollaborator = m_vCollaborators[i];
			UT_continue_if_fail(pCollaborator);

			UT_DEBUGMSG(("Pushing packet to collaborator with name: %s\n", pCollaborator->getDescription().utf8_str()));
			AccountHandler* pHandler = pCollaborator->getHandler();
			UT_continue_if_fail(pHandler);

			// overwrite remote revision for this collaborator
			_fillRemoteRev(pPacket, pCollaborator);
			
			// send!
			bool res = pHandler->send(pPacket, pCollaborator);
			if (!res)
				UT_DEBUGMSG(("Error sending a packet!\n"));
		}
	}
}

bool AbiCollab::push(Packet* pPacket, BuddyPtr collaborator)
{
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(collaborator, false);
	AccountHandler* pHandler = collaborator->getHandler();
	UT_return_val_if_fail(pHandler, false);
	
	// record
	if (m_pRecorder)
		m_pRecorder->storeOutgoing(const_cast<const Packet*>( pPacket ), collaborator);

	// overwrite remote revision for this collaborator
	_fillRemoteRev(pPacket, collaborator);
	
	// send!
	bool res = pHandler->send(pPacket, collaborator);
	if (!res)
		UT_DEBUGMSG(("Error sending a packet to collaborator %s!\n", collaborator->getDescription().utf8_str()));

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

void AbiCollab::import(SessionPacket* pPacket, BuddyPtr collaborator)
{
	UT_DEBUGMSG(("AbiCollab::import()\n"));
	UT_return_if_fail(pPacket);
	
	if (m_bDoingMouseDrag)
	{
		// we block incoming packets while dragging the mouse; this prevents 
		// scary race conditions from occuring, like importing a 'delete image' packet
		// when you are just dragging said image around
		UT_DEBUGMSG(("We are currently dragging something around; deferring packet import until after the release!\n"));
		m_vIncomingQueue.push_back(
					std::make_pair(static_cast<SessionPacket*>(pPacket->clone()), collaborator));
		return;
	}

	// record the incoming packet
	if (m_pRecorder)
		m_pRecorder->storeIncoming(pPacket, collaborator);

	// execute an alternative packet handling path when this session is being 
	// taken over by another collaborator
	if (AbstractSessionTakeoverPacket::isInstanceOf(*pPacket))
	{
		AbstractSessionTakeoverPacket* astp = static_cast<AbstractSessionTakeoverPacket*>(pPacket);
		bool res = _handleSessionTakeover(astp, collaborator);
		if (!res)
		{
			// TODO: implement/handle an offending collaborator
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
		return;
	}

	/*
	Session packets are only allowed to come in from a collaborator when:

	1. no session takeover is in progress, or 
	2a. if this session is a slave: as long as we are in the STS_SENT_TAKEOVER_ACK
		state: this means the slave didn't receive a MasterChangeRequest or 
		SessionBuddyTransferRequest packet yet. 
		When we do receive such a packet, it means all in-transit session packets
		that were sent before the SessionTakeoverRequest packet arrived at the slave 
		have been flushed from all slaves to all slaves.
		In that case we are officially in the 'exclusive' session takeover process.
	2b. if this session is a master: until the collaborator has responded to a 
		SessionTakeoverRequest from us with a SessionTakeoverAck packet
	*/

	// TODO: implement/handle an offending collaborator
	UT_return_if_fail(
				(m_eTakeoveState == STS_NONE) || /* everyone can be in this state */
				(m_eTakeoveState == STS_SENT_TAKEOVER_ACK) || /* only a slave can be in this state */
				(m_eTakeoveState == STS_SENT_TAKEOVER_REQUEST && !_hasAckedSessionTakeover(collaborator)) /* only a master can be in this state */
			);

	// import the packet; note that it might be denied due to collisions
	maskExport();
	if (AbstractChangeRecordSessionPacket::isInstanceOf(*pPacket))
		m_pActivePacket = static_cast<const AbstractChangeRecordSessionPacket*>(pPacket);
	m_Import.import(*pPacket, collaborator);
	m_pActivePacket = NULL;
	const std::vector<Packet*>& maskedPackets = unmaskExport();
	
	if (isLocallyControlled() && maskedPackets.size() > 0)
	{
		UT_DEBUGMSG(("Forwarding message (%u packets) from %s\n", maskedPackets.size(), collaborator->getDescription().utf8_str()));
		
		// It seems we are in the center of a collaboration session.
		// It's our duty to reroute the packets to the other collaborators
		for (UT_uint32 i = 0; i < m_vCollaborators.size(); i++)
		{
			// send all masked packets during import to everyone, except to the
			// person who initialy sent us the packet
			BuddyPtr pBuddy = m_vCollaborators[i];
			UT_continue_if_fail(pBuddy);
			if (pBuddy != collaborator)
			{
				UT_DEBUGMSG(("Forwarding message from %s to %s\n", collaborator->getDescription().utf8_str(), pBuddy->getDescription().utf8_str()));
				for (std::vector<Packet*>::const_iterator cit = maskedPackets.begin(); cit != maskedPackets.end(); cit++)
				{
					Packet* maskedPacket = (*cit);
					push(maskedPacket, pBuddy);
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

void AbiCollab::initiateSessionTakeover(BuddyPtr pNewMaster)
{
	UT_return_if_fail(pNewMaster);

	// NOTE: we only allow slaves in the session takeover process
	// that are on the same account as the proposed master is. The
	// others are dropped from the session. At least for now.
	// TODO: implement me
	
	// reset any old session takeover state
	m_bProposedController = false;
	m_pProposedController = pNewMaster;
	m_mAckedSessionTakeoverBuddies.clear();
	m_mAckedMasterChangeBuddies.clear();

	SessionTakeoverRequestPacket promoteTakeoverPacket(m_sId, m_pDoc->getDocUUIDString(), true);
	SessionTakeoverRequestPacket normalTakeoverPacket(m_sId, m_pDoc->getDocUUIDString(), false);
	for (std::vector<BuddyPtr>::iterator it = m_vCollaborators.begin(); it != m_vCollaborators.end(); it++)
	{
		BuddyPtr pBuddy = *it;
		UT_continue_if_fail(pBuddy);

		AccountHandler* pHandler = pBuddy->getHandler();
		UT_continue_if_fail(pHandler);

		pHandler->send(pNewMaster == pBuddy ? &promoteTakeoverPacket : &normalTakeoverPacket, pBuddy);
	}

	m_eTakeoveState = STS_SENT_TAKEOVER_REQUEST;
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

	for (std::vector<std::pair<SessionPacket*, BuddyPtr> >::iterator it = m_vIncomingQueue.begin(); it !=  m_vIncomingQueue.end(); it++)
	{
		std::pair<SessionPacket*, BuddyPtr>& pair = *it;
		UT_continue_if_fail(pair.first && pair.second);
		
		if (pair.first && pair.second)
			import(pair.first, pair.second);
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

		DELETEP(pair.first);
	}
	m_vIncomingQueue.clear();
}

bool AbiCollab::_handleSessionTakeover(AbstractSessionTakeoverPacket* pPacket, BuddyPtr collaborator)
{
	UT_DEBUGMSG(("AbiCollab::_handleSessionTakeover()\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(collaborator, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	switch (m_eTakeoveState)
	{
		case STS_NONE:
			{
				// only the SessionTakeoverRequest packet initiates the whole
				// session takeover process
				UT_return_val_if_fail(pPacket->getClassType() == PCT_SessionTakeoverRequestPacket, false);
				// we can only allow a SessionTakeoverRequest packet from the controller
				UT_return_val_if_fail(m_pController == collaborator, false);

				SessionTakeoverRequestPacket* strp = static_cast<SessionTakeoverRequestPacket*>(pPacket);
				m_bProposedController = strp->promote();
				m_pProposedController = BuddyPtr(); // will be filled in later when we are not the one being promoted

				// inform the master that we receive the takeover initiation request
				SessionTakeoverAckPacket stap;
				collaborator->getHandler()->send(&stap, collaborator);

				m_eTakeoveState = STS_SENT_TAKEOVER_ACK;
			}
			return true;
		case STS_SENT_TAKEOVER_REQUEST:
			{
				// we only accept SessionTakeoverAck packets
				UT_return_val_if_fail(pPacket->getClassType() == PCT_SessionTakeoverAckPacket, false);
				// we can only receive SessionTakeoverAck packets when we are the master
				// and not the new proposed controller
				UT_return_val_if_fail(m_pController && m_pProposedController, false);
				// a slave should only ack once
				UT_return_val_if_fail(!_hasAckedSessionTakeover(collaborator), false);

				// handle the SessionTakeoverAck packet
				m_mAckedSessionTakeoverBuddies[collaborator] = true;

				// send the buddy list to the new proposed master if we received
				// a SessionTakeoverAck packet from all slaves
				// TODO: ... or until a timeout has expired?
				std::vector<std::string> buddyIdentifiers;
				if (_allSlavesAckedSessionTakover(buddyIdentifiers))
				{
					// transfer the list of slaves that acknowledged the session
					// takeover to the new proposed master
					SessionBuddyTransferRequestPacket sbtrp(m_sId, m_pDoc->getDocUUIDString(), buddyIdentifiers);
					// TODO: send the packet
				
					m_eTakeoveState = STS_SENT_BUDDY_TRANSFER_REQUEST;
				}
			}

			return true;
		case STS_SENT_TAKEOVER_ACK:
			// we only accept a SessionBuddyTransferRequest or MasterChangeRequest packet
			UT_return_val_if_fail(
						pPacket->getClassType() == PCT_SessionBuddyTransferRequestPacket ||
						pPacket->getClassType() == PCT_MasterChangeRequestPacket,
						false
					);
			// we only accept said packets when we are a slave
			UT_return_val_if_fail(m_pController, false);

			if (pPacket->getClassType() == PCT_SessionBuddyTransferRequestPacket)
			{
				// we only accept a SessionBuddyTransferRequest packet when we are the proposed master
				UT_return_val_if_fail(m_bProposedController, false);

				// handle the SessionBuddyTransferRequest packet
				// TODO: implement me

				return true;
			}
			else if (pPacket->getClassType() == PCT_MasterChangeRequestPacket)
			{
				// we only accept a MasterChangeRequest packet when we are not the proposed master
				UT_return_val_if_fail(!m_bProposedController, false);

				// handle the MasterChangeRequest packet
				MasterChangeRequestPacket* mcrp = static_cast<MasterChangeRequestPacket*>(pPacket);
				BuddyPtr pBuddy = pManager->constructBuddy(mcrp->getBuddyIdentifier(), collaborator);
				UT_return_val_if_fail(pBuddy, false);
				m_pProposedController = pBuddy;

				// inform the new master that we want to rejoin the session
				SessionReconnectRequestPacket srrp;
				pBuddy->getHandler()->send(&srrp, pBuddy);
				
				m_eTakeoveState = STS_SENT_SESSION_RECONNECT_REQUEST;
				return true;
			}

			return false;
		case STS_SENT_BUDDY_TRANSFER_REQUEST:
			// we only accept SessionBuddyTransferAck packets
			UT_return_val_if_fail(pPacket->getClassType() == PCT_SessionBuddyTransferAckPacket, false);
			// we can only receive a SessionBuddyTransferAck packet when we are the master
			UT_return_val_if_fail(!m_pController, false);
			// we only allow an incoming SessionBuddyTransferAck packet from the proposed new master
			// TODO: implement this

			// the proposed new master is waiting now for collaborators to connect;
			// inform all our slaves that they should reconnect
			// TODO: implement me

			m_eTakeoveState = STS_SENT_MASTER_CHANGE_REQUEST;
			break;
		case STS_SENT_BUDDY_TRANSFER_ACK:
			// we only accept incoming SessionReconnectRequest packets or a SessionTakeoverFinalize packet
			UT_return_val_if_fail(
						pPacket->getClassType() == PCT_SessionReconnectRequestPacket ||
						pPacket->getClassType() == PCT_SessionTakeoverFinalizePacket,
						false
					);
			// we only accept said packets when we are the proposed master
			UT_return_val_if_fail(m_bProposedController, false)


			if (pPacket->getClassType() == PCT_SessionReconnectRequestPacket)
			{
				// we only allow an incoming SessionReconnectRequest packet from a buddy 
				// that is in the buddy list we received from the master, and we didn't receive
				// such a packet from him before
				// TODO: implement me

				// TODO: implement me
				// ...

				// NOTE: leave us in the STS_SENT_BUDDY_TRANSFER_ACK state, as
				// more SessionReconnectRequest packets can come in
				return true;
			}
			else if (pPacket->getClassType() == PCT_SessionTakeoverFinalizePacket)
			{
				// we can only allow a SessionTakeoverFinalize packet from the controller
				UT_return_val_if_fail(m_pController == collaborator, false);
				// handle the SessionTakeoverFinalize packet
				_promoteToMaster();

				m_eTakeoveState = STS_NONE;
				return true;
			}

			return false;
		case STS_SENT_MASTER_CHANGE_REQUEST:
			// we only accept MasterChangeAck packets
			UT_return_val_if_fail(pPacket->getClassType() == PCT_MasterChangeAckPacket, false);
			// we can only receive a MasterChangeAck packet when we are the master
			UT_return_val_if_fail(!m_pController, false);
			// a slave should only ack a master change request once
			UT_return_val_if_fail(!_hasAckedMasterChange(collaborator), false);

			// handle the MasterChangeAck packet
			// TODO: implement me

			if (_allSlavesAckedMasterChange())
			{
				// the session takeover is complete; inform the new master that 
				// he can restart the session
				SessionTakeoverFinalizePacket stfp;
				// TODO: send the packet

				// the session transfer is done; time to close this session
				// TODO: implement me
			}

			return true;
		case STS_SENT_MASTER_CHANGE_ACK:
			{
				// we only accept a SessionRestart packet
				UT_return_val_if_fail(pPacket->getClassType() == PCT_SessionRestartPacket, false);
				// we only accept said packet when we are a slave
				UT_return_val_if_fail(m_pController, false);
				// we only accept said packet when we are not the proposed master
				UT_return_val_if_fail(!m_bProposedController, false);
				// we only accept said packet from the proposed master
				UT_return_val_if_fail(m_pProposedController == collaborator, false);

				// handle the SessionRestart packet
				SessionRestartPacket* srp = static_cast<SessionRestartPacket*>(pPacket);
				// Nuke the current collaboration state, and restart with the
				// given revision from the proposed master
				_restartSession(m_pProposedController, srp->getDocUUID(), srp->getRev());

				m_eTakeoveState = STS_NONE;
			}
			return true;
		case STS_SENT_SESSION_RECONNECT_REQUEST:
			{
				// we only accept a SessionReconnectAck packet
				UT_return_val_if_fail(pPacket->getClassType() == PCT_SessionReconnectAckPacket, false);
				// we only accept said packet when we are a slave
				UT_return_val_if_fail(m_pController, false);
				// we only accept said packet when we are not the proposed master
				UT_return_val_if_fail(!m_bProposedController, false);
				// we only accept said packet from the proposed master
				UT_return_val_if_fail(m_pProposedController == collaborator, false);

				// inform the master that the proposed slave has accepted us
				MasterChangeAckPacket mcrp;
				m_pController->getHandler()->send(&mcrp, m_pController);

				m_eTakeoveState = STS_SENT_MASTER_CHANGE_ACK;
			}
			return true;
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
	}

	return false;
}

bool AbiCollab::_hasAckedSessionTakeover(BuddyPtr collaborator)
{
	std::map<BuddyPtr, bool>::iterator it = m_mAckedSessionTakeoverBuddies.find(collaborator);
	if (it == m_mAckedSessionTakeoverBuddies.end())
		return false;
	return (*it).second;
}

bool AbiCollab::_allSlavesAckedSessionTakover(std::vector<std::string>& buddyIdentifiers)
{	
	// FIXME: what happens when someone leaves during the session takeover
	// process? We should probably add a timeout, or some other signal to
	// not make us wait forever
	bool res = m_mAckedSessionTakeoverBuddies.size() == m_vCollaborators.size();
	if (res)
	{
		buddyIdentifiers.clear();
		for (std::vector<BuddyPtr>::iterator it = m_vCollaborators.begin(); it != m_vCollaborators.end(); it++)
		{
			BuddyPtr pBuddy = *it;
			UT_continue_if_fail(pBuddy);
			buddyIdentifiers.push_back(pBuddy->getDescriptor().utf8_str());
		}
	}
	return res;
}

bool AbiCollab::_hasAckedMasterChange(BuddyPtr collaborator)
{
	std::map<BuddyPtr, bool>::iterator it = m_mAckedMasterChangeBuddies.find(collaborator);
	if (it == m_mAckedMasterChangeBuddies.end())
		return false;
	return (*it).second;
}

bool AbiCollab::_allSlavesAckedMasterChange()
{
	// FIXME: what happens when someone leaves during the session takeover
	// process? We should probably add a timeout, or some other signal to
	// not make us wait forever
	return (m_vCollaborators.size() == 0) || 
		(m_mAckedMasterChangeBuddies.size() == m_vCollaborators.size() - 1 /* the proposed new master should not ack */);
}

void AbiCollab::_restartSession(BuddyPtr pController, const UT_UTF8String& sDocUUID, UT_sint32 iRev)
{
	m_Import.slaveInit(pController, iRev);
	m_Export.slaveInit(sDocUUID, iRev);
}

void AbiCollab::_promoteToMaster()
{
	m_Import.masterInit();
	m_Export.masterInit();

	// inform everyone that we can restart this session
	SessionRestartPacket srp(m_sId, m_pDoc->getDocUUIDString(), m_pDoc->getCRNumber());
	for (std::vector<BuddyPtr>::iterator it = m_vCollaborators.begin(); it != m_vCollaborators.end(); it++)
	{
		BuddyPtr pBuddy = *it;
		UT_continue_if_fail(pBuddy);

		AccountHandler* pHandler = pBuddy->getHandler();
		UT_continue_if_fail(pHandler);

		pHandler->send(&srp, pBuddy);
	}
}
