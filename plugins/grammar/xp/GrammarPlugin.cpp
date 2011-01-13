

#include "xap_Module.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Menu_Layouts.h"
#include "xap_DialogFactory.h"
#include "fv_View.h"
#include "ap_Menu_Id.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_EditMethod.h"
#include "DialogGrammar.h"


ABI_PLUGIN_DECLARE(Grammar)

//XAP_Menu_Id grammarId;
XAP_Dialog_Id dlgId;

static const char* Grammar_MenuLabel = "Check Grammar";
static const char* Grammar_MenuTooltip = "Checks grammar of the document.";

bool Grammar_invoke(AV_View* /*v*/, EV_EditMethodCallData * /*d*/);

// Adds "Grammar Checking" option to Abiword's Tools Menu
static void
Grammar_addToMenus(void) 
{
	// First we need to get a pointer to the application itself.
    XAP_App *pApp = XAP_App::getApp();

	XAP_DialogFactory * pDialogFactory
			= static_cast<XAP_DialogFactory *>(pApp->getDialogFactory());

	dlgId = pDialogFactory->registerDialog(DialogGrammar_Constructor, XAP_DLGT_NON_PERSISTENT);

	// Create an EditMethod that will link our method's name with
    // it's callback function.  This is used to link the name to 
    // the callback.
    EV_EditMethod *myEditMethodGrammar = new EV_EditMethod(
        "Grammar_invoke",  // name of callback function
        Grammar_invoke,    // callback function itself.
        0,                      // no additional data required.
        ""                      // description -- allegedly never used for anything
    );

	 // Now we need to get the EditMethod container for the application.
    // This holds a series of Edit Methods and links names to callbacks.
    EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();
    
    // We have to add our EditMethod to the application's EditMethodList
    // so that the application will know what callback to call when a call
    // to "Grammar_invoke" is received.
    pEMC->addEditMethod(myEditMethodGrammar);
  

    // Now we need to grab an ActionSet.  This is going to be used later
    // on in our for loop.  Take a look near the bottom.
    EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();

    
    // We need to go through and add the menu element to each "frame" 
    // of the application.  We can iterate through the frames by doing
    // XAP_App::getFrameCount() to tell us how many frames there are,
    // then calling XAP_App::getFrame(i) to get the i-th frame.

    int frameCount = pApp->getFrameCount();
    XAP_Menu_Factory * pFact = pApp->getMenuFactory();
	

	//
	// Put it in the context menu.
	//
    XAP_Menu_Id grammarId = pFact->addNewMenuAfter("contextText", NULL, "Bullets and &Numbering",EV_MLF_Normal);
    pFact->addNewLabel(NULL, grammarId, Grammar_MenuLabel, Grammar_MenuTooltip);
	//
	// Also put it under word Wount in the main menu,
	//
    pFact->addNewMenuAfter("Main", NULL, "&Word Count", EV_MLF_Normal, grammarId);

    // Create the Action that will be called.
    EV_Menu_Action* myGrammarAction = new EV_Menu_Action(
			grammarId,              // id that the layout said we could use
			0,                      // no, we don't have a sub menu.
			1,                      // yes, we raise a dialog.
			0,                      // no, we don't have a checkbox.
			0,
			"Grammar_invoke",  // name of callback function to call.
			NULL,                   // don't know/care what this is for
			NULL                    // don't know/care what this is for
        );

    // Now what we need to do is add this particular action to the ActionSet
    // of the application.  This forms the link between our new ID that we 
    // got for this particular frame with the EditMethod that knows how to 
    // call our callback function.  

    pActionSet->addAction(myGrammarAction);
    
    for (int i = 0;i < frameCount;++i)
    {
		// Get the current frame that we're iterating through.
		XAP_Frame* pFrame = pApp->getFrame(i);
		pFrame->rebuildMenus();
    }
}

static void
Grammar_RemoveFromMenus(void)
{
	// First we need to get a pointer to the application itself.
	XAP_App *pApp = XAP_App::getApp();

	// remove the edit method
	EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer() ;
	EV_EditMethod * pEM = ev_EditMethod_lookup( "Grammar_invoke" ) ;
	pEMC->removeEditMethod ( pEM ) ;
	DELETEP( pEM ) ;

	// now remove crap from the menus
	int frameCount = pApp->getFrameCount();
	XAP_Menu_Factory * pFact = pApp->getMenuFactory();

	pFact->removeMenuItem("Main", NULL, Grammar_MenuLabel);
	pFact->removeMenuItem("contextText", NULL, Grammar_MenuLabel);
	
	for (int i = 0;i < frameCount;++i)
		{
		// Get the current frame that we're iterating through.
		XAP_Frame* pFrame = pApp->getFrame(i);
		pFrame->rebuildMenus();
		}
}

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------
    
ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
    mi->name = "AbiGrammar";
    mi->desc = "Provides grammar checking to Abiword";
    mi->version = "2.9.0"; // FIXME
    mi->author = "Gabriel Bakiewicz <gabriel.bakiewicz@gmail.com>";
    mi->usage = "No Usage";
    
    // Add Grammar to AbiWord's menus.
    Grammar_addToMenus();
    
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

    Grammar_RemoveFromMenus ();

    return 1;
}


ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
    return 1; 
}


static void s_TellGrammarDone(XAP_Frame * pFrame, bool bIsSelection)
{
	pFrame->showMessageBox(bIsSelection ? "Grammar checking complete" : "Grammar checking complete",
			       XAP_Dialog_MessageBox::b_O,
			       XAP_Dialog_MessageBox::a_OK);
}

// Grammar_invoke
// -------------------
//   This is the function that we actually call to invoke the grammar checking UI.
bool Grammar_invoke(AV_View* /*v*/, EV_EditMethodCallData * /*d*/)
{
	// Get the current view that the user is in.
    XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
    //FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());
	
	//UT_return_val_if_fail(pFrame, false);
	//pFrame->raise();

	XAP_DialogFactory * pDialogFactory
			= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());
	UT_return_val_if_fail(pDialogFactory, false);

	DialogGrammar * pDialog
			= static_cast<DialogGrammar *>(pDialogFactory->requestDialog(dlgId));
	UT_return_val_if_fail (pDialog, false);

	// run the dialog (it probably should be modeless if anyone
	// gets the urge to make it safe that way)
	pDialog->runModal(pFrame);
	
	bool bOK = pDialog->isComplete();
	if (bOK)
	   s_TellGrammarDone(pFrame, pDialog->isSelection());

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}


