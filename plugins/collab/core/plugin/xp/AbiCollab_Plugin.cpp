/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiCollab- Code to enable the modification of remote documents.
 * Copyright (C) 2005 by Martin Sevior
 * Copyright (C) 2006,2007 by Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2007 by One Laptop Per Child
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

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_abicollab_register
#define abi_plugin_unregister abipgn_abicollab_unregister
#define abi_plugin_supports_version abipgn_abicollab_supports_version
#endif

#include "xap_Module.h"
#include "xap_App.h"
#include "xap_Frame.h"
// needed for menu's
#include "ap_Menu_Id.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_EditMethod.h"
#include "xap_Menu_Layouts.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "ut_vector.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_FileOpenSaveAs.h"

#include <dialogs/xp/ap_Dialog_CollaborationShare.h>
#include <dialogs/xp/ap_Dialog_CollaborationJoin.h>
#include <dialogs/xp/ap_Dialog_CollaborationAccounts.h>

#include <session/xp/AbiCollabSessionManager.h>
#include <session/xp/AbiCollab.h>
#include <session/xp/DiskSessionRecorder.h>

#include "AbiCollab_Command.h"
#include "AbiCollab_Plugin.h"

// forward declarations
static void s_abicollab_add_menus();
static void s_abicollab_remove_menus();
#if defined(ABICOLLAB_RECORD_ALWAYS) && defined(DEBUG)
static void s_cleanup_old_sessions();
#endif


// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------

ABI_PLUGIN_DECLARE(AbiCollab)
  
ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	mi->name = "AbiWord Collaboration";
	mi->desc = "This plugin allows real-time collaborative document editing";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Martin Sevior <msevior@physics.unimelb.edu.au>\nMarc Maurer <uwog@uwog.net>\nMarc Oude Kotte <foddex@foddex.net>";
	mi->usage = "com.abisource.abiword.abicollab.command";
	
	s_abicollab_add_menus();
	
	// On Windows, we must share our HMODULE/HINSTANCE so we can do gui
	#ifdef WIN32
	AbiCollabSessionManager::getManager()->setInstance((HINSTANCE)s_hModule);
	#endif
	
	// register all available account handlers
	AbiCollabSessionManager::getManager()->registerAccountHandlers();

	// finally, register all dialogs we'll use as well    
   	AbiCollabSessionManager::getManager()->registerDialogs();
   	
   	// load the settings from the profile
   	AbiCollabSessionManager::getManager()->loadProfile();
	
	// remove old sessions
#if defined(ABICOLLAB_RECORD_ALWAYS) && defined(DEBUG)
	s_cleanup_old_sessions();
#endif
   		
	return 1;
}


ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;
	
	s_abicollab_remove_menus();
	
	// kill all active sessions
	AbiCollabSessionManager::getManager()->disconnectSessions();
	
	// store the settings to the profile
	AbiCollabSessionManager::getManager()->storeProfile();

	// stop all active accounts
	AbiCollabSessionManager::getManager()->destroyAccounts();

	// unregister all available account handlers
	AbiCollabSessionManager::getManager()->unregisterAccountHandlers();

	// unregister all import/export sniffers we have registered
	AbiCollabSessionManager::getManager()->unregisterSniffers();
	
	// unregister all dialogs we use
	AbiCollabSessionManager::getManager()->unregisterDialogs();

	return 1;
}


ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
	return 1; 
}


// -----------------------------------------------------------------------
//
//      Abiword Plugin Menu Structure
//
// -----------------------------------------------------------------------

// FIXME: make these translatable strings
static const char * szCollaboration = "&Collaborate";
static const char * szCollaborationTip = "Collaborate over the internet or local network";

static const char * szCollaborationOffer = "Share Document";
static const char * szCollaborationOfferTip = "Offer the current document for collaboration";

static const char * szCollaborationJoin = "Open Shared Document";
static const char * szCollaborationJoinTip = "Open a shared document";

static const char * szCollaborationAccounts = "Accounts";
static const char * szCollaborationAccountsTip = "Manage collaboration accounts";

static const char * szCollaborationShowAuthors = "Show Authors";
static const char * szCollaborationShowAuthorsTip = "Show who wrote each piece of text by with different colors";

#if defined(DEBUG)
#if !defined(ABICOLLAB_RECORD_ALWAYS)
static const char * szCollaborationRecord = "Record this Session";
static const char * szCollaborationRecordTip = "Record a session to disk, for debugging purposes";
#endif
static const char * szCollaborationViewRecord = "View Session";
static const char * szCollaborationViewRecordTip = "Load a recorded session from disk and show the packets";
#endif

static const char * szEndCollaboration = "EndCollaboration";

// some function prototypes
static bool s_abicollab_offer(AV_View* v, EV_EditMethodCallData *d);
static bool s_abicollab_join(AV_View* v, EV_EditMethodCallData *d);
static bool s_abicollab_accounts(AV_View* v, EV_EditMethodCallData *d);
static bool s_abicollab_authors(AV_View* v, EV_EditMethodCallData *d);
#ifdef DEBUG
static bool s_abicollab_record(AV_View* v, EV_EditMethodCallData *d);
static bool s_abicollab_viewrecord(AV_View* v, EV_EditMethodCallData *d);
#endif
static bool s_abicollab_command_invoke(AV_View* v, EV_EditMethodCallData *d);
#define ABIWORD_VIEW  	FV_View * pView = static_cast<FV_View *>(pAV_View)

/*!
 * returns true if at least one account is online
 */
static bool s_any_accounts_online(bool bIncludeNonManualShareAccounts = true)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);
	
	const std::vector<AccountHandler *>& vecAccounts = pManager->getAccounts();
	
	for (UT_uint32 i = 0; i < vecAccounts.size(); i++)
	{
		AccountHandler* pHandler = vecAccounts[i];
		if (pHandler && pHandler->isOnline())
		{
			if (bIncludeNonManualShareAccounts)
				return true;
			if (pHandler->canManuallyStartSession())
				return true;
		}
	}
	return false;
}
/*!
 * returns checked true if current document is marked for show authors
 */
Defun_EV_GetMenuItemState_Fn(collab_GetState_ShowAuthors)
{
	UT_UNUSED(id);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, EV_MIS_Gray);
	
	if (!s_any_accounts_online())
		return EV_MIS_Gray;

	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	PD_Document* pDoc = pView->getDocument();
	UT_return_val_if_fail (pDoc, EV_MIS_Gray);
	if (!pManager->isInSession(pDoc))
	{
		return EV_MIS_Gray;
	}
	if(pDoc->isShowAuthors())
	{
		return EV_MIS_Toggled;
	}
	return EV_MIS_ZERO;
}

/*!
 * returns checked true if currently recording
 */
Defun_EV_GetMenuItemState_Fn(collab_GetState_Recording)
{
	UT_UNUSED(id);

	// only do this in debug mode
#if !defined(ABICOLLAB_RECORD_ALWAYS) && defined(DEBUG)
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, EV_MIS_Gray);
	
	if (!s_any_accounts_online())
		return EV_MIS_Gray;
	
	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);
	PD_Document* pDoc = pView->getDocument();
	UT_return_val_if_fail (pDoc, EV_MIS_Gray);
	
	// retrieve session
	AbiCollab* session = pManager->getSession( pDoc );
	if (session)
	{
		if (session->isRecording())
			return EV_MIS_Toggled;
		else
			return EV_MIS_ZERO;
	}
	
	// not a session
	return EV_MIS_Gray;
#else
	UT_UNUSED(pAV_View);
	return EV_MIS_Gray;
#endif
}

/*!
 * returns grayed if there are no active connections
 */
Defun_EV_GetMenuItemState_Fn(collab_GetState_AnyActive)
{
	UT_UNUSED(pAV_View);
	UT_UNUSED(id);

	if (!s_any_accounts_online())
		return EV_MIS_Gray;
	return EV_MIS_ZERO;
}

Defun_EV_GetMenuItemState_Fn(collab_GetState_CanShare)
{
	UT_UNUSED(id);

	ABIWORD_VIEW;
	UT_return_val_if_fail (pView, EV_MIS_Gray);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, EV_MIS_Gray);

	// you can't share a document when no account is online
	if (!s_any_accounts_online(false))
		return EV_MIS_Gray;

	// you can open the share dialog when the document is not shared yet, or
	// when it is shared and it is 'owned' locally
	
	PD_Document* pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, EV_MIS_Gray);
	
	AbiCollab* session = pManager->getSession(pDoc);
	if (session)
	{
		if (session->isLocallyOwned())
			return EV_MIS_ZERO;
		else
			return EV_MIS_Gray;
	}

	// the document is not shared yet, but there are active accounts...
	return EV_MIS_ZERO;
}



/*!
 * This implements the "Collaborate" main submenu.
 */
void s_abicollab_add_menus()
{
    // First we need to get a pointer to the application itself.
    XAP_App *pApp = XAP_App::getApp();
    EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();
    int frameCount = pApp->getFrameCount();
    XAP_Menu_Factory * pFact = pApp->getMenuFactory();    
    EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();
	
	// TODO: make this a translatable set of strings
	// const XAP_StringSet * pSS = pApp->getStringSet();
    
	// The Collaboration menu item
	XAP_Menu_Id collabId = pFact->addNewMenuBefore("Main", NULL, AP_MENU_ID_WINDOW, EV_MLF_BeginSubMenu);
    pFact->addNewLabel(NULL, collabId, szCollaboration, szCollaborationTip);
    EV_Menu_Action* myCollaborationAction = new EV_Menu_Action (
		collabId,    			 // id that the layout said we could use
		1,                      // yes, we have a sub menu.
		0,                      // no, we don't raise a dialog.
		0,                      // no, we don't have a checkbox.
		0,                      // no radio buttons for me, thank you
		NULL,                   // no callback function to call.
		NULL,                   // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myCollaborationAction);

	// The Start Collaboration connect item
	XAP_Menu_Id collabOfferId = pFact->addNewMenuAfter("Main", NULL, collabId, EV_MLF_Normal);
    pFact->addNewLabel(NULL, collabOfferId, szCollaborationOffer, szCollaborationOfferTip);
	EV_Menu_Action* myActionOffer = new EV_Menu_Action (
		collabOfferId,   	  // id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		1,                      // yes, we raise a dialog.
		0,                      // no, we don't have a checkbox.
		0,                      // no radio buttons for me, thank you
		"s_abicollab_offer",    // name of callback function to call.
		collab_GetState_CanShare,  // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myActionOffer);
	EV_EditMethod *myEditMethodOffer = new EV_EditMethod (
		"s_abicollab_offer",    // name of callback function
		s_abicollab_offer,      // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(myEditMethodOffer);

	// The Join Collaboration connect item
	XAP_Menu_Id collabJoinId = pFact->addNewMenuAfter("Main", NULL, collabOfferId, EV_MLF_Normal);
    pFact->addNewLabel(NULL, collabJoinId, szCollaborationJoin, szCollaborationJoinTip);
	EV_Menu_Action* myActionJoin = new EV_Menu_Action (
		collabJoinId,   		// id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		1,                      // yes, we raise a dialog.
		0,                      // no, we don't have a checkbox.
		0,                      // no radio buttons for me, thank you
		"s_abicollab_join",     // name of callback function to call.
		collab_GetState_AnyActive,  // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myActionJoin);
	EV_EditMethod *myEditMethodJoin = new EV_EditMethod (
		"s_abicollab_join",     // name of callback function
		s_abicollab_join,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(myEditMethodJoin);

	// The Join Collaboration connect item
	XAP_Menu_Id collabAccountsId = pFact->addNewMenuAfter("Main", NULL, collabJoinId, EV_MLF_Normal);
    pFact->addNewLabel(NULL, collabAccountsId, szCollaborationAccounts, szCollaborationAccountsTip);
	EV_Menu_Action* myActionAccounts = new EV_Menu_Action (
		collabAccountsId,   		// id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		1,                      // yes, we raise a dialog.
		0,                      // no, we don't have a checkbox.
		0,                      // no radio buttons for me, thank you
		"s_abicollab_accounts",     // name of callback function to call.
		NULL,                   // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myActionAccounts);
	EV_EditMethod *myEditMethodAccounts = new EV_EditMethod (
		"s_abicollab_accounts",     // name of callback function
		s_abicollab_accounts,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(myEditMethodAccounts);


   
    
	// The Show Authors item
	XAP_Menu_Id ShowAuthorId = pFact->addNewMenuAfter("Main", NULL, collabAccountsId, EV_MLF_Normal);
    pFact->addNewLabel(NULL, ShowAuthorId,  szCollaborationShowAuthors,  szCollaborationShowAuthorsTip);
	EV_Menu_Action* myActionShowAuthors = new EV_Menu_Action (
		ShowAuthorId,   	  // id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		0,                      // no, we don't raise a dialog.
		1,                      // yes, we have a checkbox.
		0,                      // no radio buttons for me, thank you
		"s_abicollab_authors",    // name of callback function to call.
		collab_GetState_ShowAuthors, // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myActionShowAuthors);
	EV_EditMethod *myEditMethodShowAuthors = new EV_EditMethod (
		"s_abicollab_authors",    // name of callback function
		s_abicollab_authors,      // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(myEditMethodShowAuthors);

	// The Record session connect item
#if defined(DEBUG)

#if !defined(ABICOLLAB_RECORD_ALWAYS)
	XAP_Menu_Id collabRecordId = pFact->addNewMenuAfter("Main", NULL,ShowAuthorId , EV_MLF_Normal);
    pFact->addNewLabel(NULL, collabRecordId, szCollaborationRecord, szCollaborationRecordTip);
	EV_Menu_Action* myActionRecord = new EV_Menu_Action (
		collabRecordId,   		// id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		0,                      // no, we don't raise a dialog.
		1,                      // yes, we have a checkbox.
		0,                      // no radio buttons for me, thank you
		"s_abicollab_record",    // name of callback function to call.
		collab_GetState_Recording, // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myActionRecord);
	EV_EditMethod *myEditMethodRecord = new EV_EditMethod (
		"s_abicollab_record",     // name of callback function
		s_abicollab_record,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(myEditMethodRecord);
	
	XAP_Menu_Id followupMenuId = collabRecordId;
#else
	XAP_Menu_Id followupMenuId = ShowAuthorId;
#endif /* !defined(ABICOLLAB_RECORD_ALWAYS) */
	
	XAP_Menu_Id collabViewRecordId = pFact->addNewMenuAfter("Main", NULL, followupMenuId, EV_MLF_Normal);
    pFact->addNewLabel(NULL, collabViewRecordId, szCollaborationViewRecord, szCollaborationViewRecordTip);
	EV_Menu_Action* myActionViewRecord = new EV_Menu_Action (
		collabViewRecordId,   	// id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		1,                      // yes, we don't raise a dialog.
		0,                      // no, we have a checkbox.
		0,                      // no radio buttons for me, thank you
		"s_abicollab_viewrecord",   // name of callback function to call.
		NULL,					// Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
	pActionSet->addAction(myActionViewRecord);
	EV_EditMethod *myEditMethodViewRecord = new EV_EditMethod (
		"s_abicollab_viewrecord",     // name of callback function
		s_abicollab_viewrecord,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(myEditMethodViewRecord);
	
	XAP_Menu_Id lastMenuId = collabViewRecordId;
#else
	XAP_Menu_Id lastMenuId = ShowAuthorId;
#endif /* defined(DEBUG) */

	// End of the Collaboration menu
	XAP_Menu_Id endCollaborationId = pFact->addNewMenuAfter("Main", NULL, lastMenuId, EV_MLF_EndSubMenu);
	pFact->addNewLabel(NULL, endCollaborationId, szEndCollaboration, NULL);
	EV_Menu_Action* myEndCollaborationAction = new EV_Menu_Action (
		endCollaborationId,     // id that the layout said we could use
		0,                      // no, we don't have a sub menu.
		0,                      // no, we raise a dialog.
		0,                      // no, we don't have a checkbox.
		0,                      // no radio buttons for me, thank you
		NULL,                   // name of callback function to call.
		NULL,                   // Function for whether not label is enabled/disabled checked/unchecked
		NULL                    // Function to compute Menu Label "Dynamic Label"
	);
    pActionSet->addAction(myEndCollaborationAction);
    
	EV_EditMethod* myCommandEM = new EV_EditMethod ("com.abisource.abiword.abicollab.command", s_abicollab_command_invoke, 0, "" );
	pEMC->addEditMethod (myCommandEM);
	
    // We need to go through and add the menu element to each "frame" 
    // of the application.  We can iterate through the frames by doing
    // XAP_App::getFrameCount() to tell us how many frames there are,
    // then calling XAP_App::getFrame(i) to get the i-th frame.
    for(int i = 0; i < frameCount;++i)
    {
        // Get the current frame that we're iterating through.
		XAP_Frame* pFrame = pApp->getFrame(i);
		pFrame->rebuildMenus();
    }
}

/*!
 * Remove the menu items unpon unloading the plugin.
 */
void s_abicollab_remove_menus()
{
	// First we need to get a pointer to the application itself.
	XAP_App *pApp = XAP_App::getApp();
	
	// remove the edit method
	EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();
	EV_EditMethod * pEM;

	pEM = ev_EditMethod_lookup ( "s_abicollab_offer" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;

	pEM = ev_EditMethod_lookup ( "s_abicollab_join" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;

	pEM = ev_EditMethod_lookup ( "s_abicollab_accounts" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;

	pEM = ev_EditMethod_lookup ( "s_abicollab_authors" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;
	
#if !defined(ABICOLLAB_RECORD_ALWAYS) && defined(DEBUG)
	pEM = ev_EditMethod_lookup ( "s_abicollab_record" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;
#endif
	
#if defined(DEBUG)
	pEM = ev_EditMethod_lookup ( "s_abicollab_viewrecord" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;
#endif
	
	pEM = ev_EditMethod_lookup ( "com.abisource.abiword.abicollab.command" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;

	// now remove crap from the menus
	int frameCount = pApp->getFrameCount();
	XAP_Menu_Factory * pFact = pApp->getMenuFactory();

	pFact->removeMenuItem("Main", NULL, szCollaboration);
	pFact->removeMenuItem("Main", NULL, szCollaborationOffer);
	pFact->removeMenuItem("Main", NULL, szCollaborationJoin);
	pFact->removeMenuItem("Main", NULL, szCollaborationAccounts);	
	pFact->removeMenuItem("Main", NULL, szCollaborationShowAuthors);	
#if !defined(ABICOLLAB_RECORD_ALWAYS) && defined(DEBUG)
	pFact->removeMenuItem("Main", NULL, szCollaborationRecord);	
#endif
#if defined(DEBUG)
	pFact->removeMenuItem("Main", NULL, szCollaborationViewRecord);
#endif
	pFact->removeMenuItem("Main", NULL, szEndCollaboration);
	
	for (int i = 0; i < frameCount; ++i)
	{
		// Get the current frame that we're iterating through.
		XAP_Frame* pFrame = pApp->getFrame(i);
		pFrame->rebuildMenus();
	}
}

bool s_abicollab_offer(AV_View* /*v*/, EV_EditMethodCallData* /*d*/)
{
	UT_DEBUGMSG(("s_abicollab_offer\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);
	
	// Get the current view that the user is in.
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	// Get an Accounts dialog instance
	XAP_DialogFactory* pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	UT_return_val_if_fail(pFactory, false);
	AP_Dialog_CollaborationShare* pDialog = static_cast<AP_Dialog_CollaborationShare*>(
				pFactory->requestDialog(AbiCollabSessionManager::getManager()->getDialogShareId())
			);
	// Run the dialog
	pDialog->runModal(pFrame);
	// Handle the dialog outcome
	AP_Dialog_CollaborationShare::tAnswer answer = pDialog->getAnswer();
	
	switch (answer)
	{
		case AP_Dialog_CollaborationShare::a_OK:
			{
				AccountHandler* pAccount = pDialog->getAccount();
				const std::vector<std::string> vAcl = pDialog->getAcl();
				// TODO: move the share() function to the AbiCollabSessionManager
				pDialog->share(pAccount, vAcl);
			}
			break;
		case AP_Dialog_CollaborationShare::a_CANCEL:
			break;
		default:
			UT_ASSERT_HARMLESS(UT_NOT_REACHED);
			break;
	}	

	// Delete the dialog
	pFactory->releaseDialog(pDialog);

	return true;
}

bool s_abicollab_authors(AV_View* v, EV_EditMethodCallData* /*d*/)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);
	FV_View * pView = static_cast<FV_View *>(v);
	PD_Document * pDoc = pView->getDocument();
	bool b = pDoc->isShowAuthors();
	pDoc->setShowAuthors(!b);
	return true;
}

bool s_abicollab_join(AV_View* /*v*/, EV_EditMethodCallData* /*d*/)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);
	
	// Get the current view that the user is in.
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	// Get an Accounts dialog instance
	XAP_DialogFactory* pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	UT_return_val_if_fail(pFactory, false);
	AP_Dialog_CollaborationJoin* pDialog = static_cast<AP_Dialog_CollaborationJoin*>(
				pFactory->requestDialog(AbiCollabSessionManager::getManager()->getDialogJoinId())
			);
	// Run the dialog
	pDialog->runModal(pFrame);
	// Handle the dialog outcome
	AP_Dialog_CollaborationJoin::tAnswer answer = pDialog->getAnswer();
	BuddyPtr pBuddy = pDialog->getBuddy();
	DocHandle* pDocHandle = pDialog->getDocHandle();
	pFactory->releaseDialog(pDialog);
	
	switch (answer)
	{
		case AP_Dialog_CollaborationJoin::a_OPEN:
			{
				UT_return_val_if_fail(pBuddy && pDocHandle, false);
				// Check if we have already joined this session. If so, then just
				// ignore the request. Otherwise actually join the session.
				AbiCollab* pSession = pManager->getSessionFromSessionId(pDocHandle->getSessionId());
				if (pSession)
				{
					UT_DEBUGMSG(("Already connected to session, just raising the associated frame\n"));

					// Just raise a frame that contains this session, instead of
					// opening the document again
					XAP_Frame* pFrameForSession = pManager->findFrameForSession(pSession);
					UT_return_val_if_fail(pFrameForSession, false);
					pFrameForSession->raise();
				}
				else
					pManager->joinSessionInitiate(pBuddy, pDocHandle);	
			}
			break;
		case AP_Dialog_CollaborationJoin::a_CANCEL:
			break;
	}	
	
	return true;
}

bool s_abicollab_accounts(AV_View* /*v*/, EV_EditMethodCallData* /*d*/)
{
	// Get the current view that the user is in.
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	// Get an Accounts dialog instance
	XAP_DialogFactory* pFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	UT_return_val_if_fail(pFactory, false);
	AP_Dialog_CollaborationAccounts* pDialog = static_cast<AP_Dialog_CollaborationAccounts*>(
				pFactory->requestDialog(AbiCollabSessionManager::getManager()->getDialogAccountsId())
			);
	// Run the dialog
	pDialog->runModal(pFrame);
	pFactory->releaseDialog(pDialog);
	return true;
}

#ifdef DEBUG
bool s_abicollab_record(AV_View* /*v*/, EV_EditMethodCallData* /*d*/)
{
	UT_DEBUGMSG(("s_abicollab_record\n"));
	// this option only works in debug mode
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, false);
	PD_Document* pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
	UT_return_val_if_fail(pDoc, false);
	
	// retrieve session
	AbiCollab* session = pManager->getSession( pDoc );
	if (session)
	{
		if (session->isRecording()) {
			session->stopRecording();
			UT_ASSERT(!session->isRecording());
		} else {
			session->startRecording( new DiskSessionRecorder( session ) );
			UT_ASSERT(session->isRecording());
		}
	}
	return true;
}

bool s_abicollab_viewrecord(AV_View* /*v*/, EV_EditMethodCallData* /*d*/)
{
	UT_DEBUGMSG(("s_abicollab_viewrecord\n"));
	
	// ask user what file to open
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	XAP_DialogFactory * pDialogFactory = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	XAP_Dialog_FileOpenSaveAs * pDialog = static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_FILE_OPEN));
	UT_return_val_if_fail (pDialog, false);
	pDialog->setSuggestFilename(false);
	pDialog->runModal( pFrame );
	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const std::string & resultPathname = pDialog->getPathname();
		if (!resultPathname.empty())
		{
			UT_DEBUGMSG(("filename = '%s'\n", resultPathname.c_str()));
			DiskSessionRecorder::dumpSession(resultPathname);
		} 
		else 
		{
			UT_DEBUGMSG(("no filename selected\n"));
		}
	} 
	else 
	{
		UT_DEBUGMSG(("OK not clicked\n"));
	}
	pDialogFactory->releaseDialog(pDialog);
	return true;
}
#endif

bool s_abicollab_command_invoke(AV_View* /*v*/, EV_EditMethodCallData *d)
{
	UT_DEBUGMSG(("s_abicollab_command_invoke()\n"));

	UT_UTF8String argv(d->m_pData, d->m_dataLength);
	UT_DEBUGMSG(("command line arguments: %s\n", argv.utf8_str()));
	
	AbiCollab_Command command(argv);
	if (command.execute())
	{
		UT_DEBUGMSG(("AbiCollab command executed successful\n"));
	}
	else
	{
		UT_DEBUGMSG(("AbiCollab command failed to execute successfully\n"));
	}

	return true;
}

#if defined(ABICOLLAB_RECORD_ALWAYS) && defined(DEBUG)

#ifndef WIN32

#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*!
 * returns all found session files that are older than the specified date
 * TODO: needs win32 implementation using FindFirstFile
 */
void findSessionFiles( time_t now, std::vector<std::string>& files )
{
	const int secondsInDay = 24*60*60;
	const char* prefix = DiskSessionRecorder::getPrefix();
	size_t prefixLen = strlen(prefix);
	
	struct dirent** namelist;
	int n = scandir( DiskSessionRecorder::getTargetDirectory(), &namelist, NULL, alphasort );
	UT_DEBUGMSG(("findSessionFiles: got %d files in %s\n", n, DiskSessionRecorder::getTargetDirectory()));
	for (int i=0; i<n; ++i)
	{
		UT_DEBUGMSG(("findSessionFiles: considering %s\n", namelist[i]->d_name));
		
		// construct full name so we can stat this node
		std::string fullname = DiskSessionRecorder::getTargetDirectory();
		fullname += '/';
		fullname += namelist[i]->d_name;
		
		// stat node
		struct stat details;
		if (stat( fullname.c_str(), &details )==0)
		{
			// check if it is a file
			if (!(S_ISDIR(details.st_mode)))
			{
				//UT_DEBUGMSG(("findSessionFiles: it's a file!\n"));
				// if it is a session file
				if (!strncmp( namelist[i]->d_name, prefix, prefixLen ))
				{
					//UT_DEBUGMSG(("findSessionFiles: it's a session file!\n"));
					// and if it's too old
					UT_DEBUGMSG(("findSessionFiles: file is %u seconds old, treshold is %u!\n", now - details.st_mtime, secondsInDay));
					if ((now > details.st_mtime) &&				// is file older than current time? (should always be the case, but who knows)
						(now - details.st_mtime > secondsInDay))	// is file older than a day?
					{
						//UT_DEBUGMSG(("findSessionFiles: it's an old session file!\n"));
						files.push_back( fullname );
					}
				}
			}
		}
		
		// cleanup
		free(namelist[i]);
	}
	// cleanup
	free(namelist);
}

/*!
 * removes all sessions that are older than 24-hours
 */
void s_cleanup_old_sessions()
{
	UT_DEBUGMSG(("s_cleanup_old_sessions()\n"));
	
	// get all files we need to delete
	std::vector<std::string> files;
	findSessionFiles( time(0), files );
	
	// delete files!
	UT_DEBUGMSG(("cleanupOldSessions: removing %u files\n", files.size()));
	for (size_t i=0; i<files.size(); ++i) 
	{
		UT_DEBUGMSG(("cleanupOldSessions: removing %s\n", files[i].c_str()));
		if (unlink( files[i].c_str() )!=0)
		{
			UT_DEBUGMSG(("cleanupOldSessions: unlink failed!\n"));
		}
	}
}

#else

void cleanupOldSessions()
{
	UT_DEBUGMSG(("cleanupOldSessions: not implemented on WIN32!\n"));
}

#endif

#endif
