

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_abimathview_register
#define abi_plugin_unregister abipgn_abimathview_unregister
#define abi_plugin_supports_version abipgn_abimathview_supports_version
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AbiLasemMathView.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "ut_mbtowc.h"
#include "gr_Painter.h"
#include "gr_CairoGraphics.h"
#include "xad_Document.h"
#include "xap_Module.h"
#include "ap_App.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Module.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "ap_Menu_Id.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_EditMethod.h"
#include "xap_Menu_Layouts.h"
#include "ie_exp.h"
#include "ie_types.h"
#include "xap_Dialog_Id.h"
#include "ap_Dialog_Id.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "ap_Strings.h"
#include "ap_Dialog_Latex.h"

#include "ut_mbtowc.h"
#include "ap_Menu_Id.h"
#include "ie_math_convert.h"

#include "ut_sleep.h"
#include <goffice/goffice.h>
#include <sys/types.h>  
#include <sys/stat.h>
#ifdef TOOLKIT_WIN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "ut_files.h"
#endif

#ifndef HAVE_LSM_ITEX_TO_MATHML
/* Lasem - SVG and Mathml library
 *
 * Copyright © 2013 Emmanuel Pacaud
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author:
 * 	Emmanuel Pacaud <emmanuel@gnome.org>
 */

extern "C" char * itex2MML_parse (const char * buffer, unsigned long length);
extern "C" void   itex2MML_free_string (char * str);

static char *
lsm_itex_to_mathml (const char *itex, int size)
{
	char *mathml;

	if (itex == NULL)
		return NULL;

	if (size < 0)
		size = strlen (itex);

	mathml = itex2MML_parse (itex, size);
	if (mathml == NULL)
		return NULL;

	if (mathml[0] == '\0') {
		itex2MML_free_string (mathml);
		return NULL;
	}

	return mathml;
}

static void
lsm_itex_free_mathml_buffer (char *mathml)
{
	if (mathml == NULL)
		return;

	itex2MML_free_string (mathml);
}
#endif

static GR_LasemMathManager * pMathManager = NULL; // single plug-in instance of GR_MathManager

//
// AbiMathView_addToMenus
// -----------------------
//   Adds "Equation" "From File" "From Latex" options to AbiWord's Main Menu.
//

static bool AbiMathView_FileInsert(AV_View* v, EV_EditMethodCallData *d);
static bool AbiMathView_LatexInsert(AV_View* v, EV_EditMethodCallData *d);

// FIXME make these translatable strings
/*
static const char * AbiMathView_MenuLabelEquation = "Equation";
static const char * AbiMathView_MenuTooltipEquation = "Insert Equation";
static const char* AbiMathView_MenuLabelFileInsert = "From File";
static const char* AbiMathView_MenuTooltipFileInsert = "Insert MathML from a file";
static const char* AbiMathView_MenuLabelLatexInsert = "From Latex";
static const char* AbiMathView_MenuTooltipLatexInsert = "Insert Equation from a Latex expression";
*/
static const char * AbiMathView_MenuLabelEquation = NULL;
static const char * AbiMathView_MenuTooltipEquation = NULL;
static const char* AbiMathView_MenuLabelFileInsert = NULL;
static const char* AbiMathView_MenuTooltipFileInsert = NULL;
static const char* AbiMathView_MenuLabelLatexInsert = NULL;
static const char* AbiMathView_MenuTooltipLatexInsert = NULL;
static const char* AbiMathView_MenuEndEquation= "EndEquation";
static XAP_Menu_Id newEquationID;
static XAP_Menu_Id FromFileID;
static XAP_Menu_Id FromLatexID;
static XAP_Menu_Id endEquationID;
static void AbiMathView_addToMenus()
{
    // First we need to get a pointer to the application itself.
    XAP_App *pApp = XAP_App::getApp();
    //
    // Translated Strings
    //
    const XAP_StringSet * pSS = pApp->getStringSet();
    AbiMathView_MenuLabelEquation= pSS->getValue(AP_STRING_ID_MENU_LABEL_INSERT_EQUATION);
    AbiMathView_MenuTooltipEquation = pSS->getValue(AP_STRING_ID_MENU_LABEL_TOOLTIP_INSERT_EQUATION);
    AbiMathView_MenuLabelFileInsert = pSS->getValue(AP_STRING_ID_MENU_LABEL_INSERT_EQUATION_FILE);
    AbiMathView_MenuTooltipFileInsert = pSS->getValue(AP_STRING_ID_MENU_LABEL_TOOLTIP_INSERT_EQUATION_FILE);
    AbiMathView_MenuLabelLatexInsert = pSS->getValue(AP_STRING_ID_MENU_LABEL_INSERT_EQUATION_LATEX);
    AbiMathView_MenuTooltipLatexInsert = pSS->getValue(AP_STRING_ID_MENU_LABEL_TOOLTIP_INSERT_EQUATION_LATEX);
    
    // Create an EditMethod that will link our method's name with
    // it's callback function.  This is used to link the name to 
    // the callback.
    EV_EditMethod *myEditMethodFile = new EV_EditMethod(
        "AbiMathView_FileInsert",  // name of callback function
        AbiMathView_FileInsert,    // callback function itself.
        0,                      // no additional data required.
        ""                      // description -- allegedly never used for anything
    );

    EV_EditMethod *myEditMethodLatex = new EV_EditMethod(
        "AbiMathView_LatexInsert",  // name of callback function
        AbiMathView_LatexInsert,    // callback function itself.
        0,                      // no additional data required.
        ""                      // description -- allegedly never used for anything
    );
   
    // Now we need to get the EditMethod container for the application.
    // This holds a series of Edit Methods and links names to callbacks.
    EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();
    
    // We have to add our EditMethod to the application's EditMethodList
    // so that the application will know what callback to call when a call
    // to "AbiMathView_FileInsert" is received.
    pEMC->addEditMethod(myEditMethodFile);
    pEMC->addEditMethod(myEditMethodLatex);
  

    // Now we need to grab an ActionSet.  This is going to be used later
    // on in our for loop.  Take a look near the bottom.
    EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();

    XAP_Menu_Factory * pFact = pApp->getMenuFactory();

// Put it after Insert Picture in the Main menu

    newEquationID= pFact->addNewMenuAfter("Main",NULL,AP_MENU_ID_INSERT_GRAPHIC,EV_MLF_BeginSubMenu);
   UT_DEBUGMSG(("newEquationID %d \n",newEquationID));


    pFact->addNewLabel(NULL,newEquationID,AbiMathView_MenuLabelEquation, AbiMathView_MenuTooltipEquation);

    // Create the Action that will be called.
    EV_Menu_Action* myEquationAction = new EV_Menu_Action(
	newEquationID,          // id that the layout said we could use
	1,                      // yes, we have a sub menu.
	0,                      // no, we don't raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	NULL,                   //  no callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );

    // Now what we need to do is add this particular action to the ActionSet
    // of the application.  This forms the link between our new ID that we 
    // got for this particular frame with the EditMethod that knows how to 
    // call our callback function.  

    pActionSet->addAction(myEquationAction);

    FromFileID= pFact->addNewMenuAfter("Main",NULL,newEquationID,EV_MLF_Normal);
    UT_DEBUGMSG(("FromFile ID %d \n",FromFileID));

    pFact->addNewLabel(NULL,FromFileID,AbiMathView_MenuLabelFileInsert, AbiMathView_MenuTooltipFileInsert);


    // Create the Action that will be called.
    EV_Menu_Action* myFileAction = new EV_Menu_Action(
	FromFileID,                     // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	1,                      // yes, we raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	"AbiMathView_FileInsert",  // name of callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );

    // Now what we need to do is add this particular action to the ActionSet
    // of the application.  This forms the link between our new ID that we 
    // got for this particular frame with the EditMethod that knows how to 
    // call our callback function.  

    pActionSet->addAction(myFileAction);

   FromLatexID= pFact->addNewMenuAfter("Main",NULL,FromFileID,EV_MLF_Normal);
   UT_DEBUGMSG(("Latex ID %d \n",FromLatexID));
 
    pFact->addNewLabel(NULL,FromLatexID,AbiMathView_MenuLabelLatexInsert, AbiMathView_MenuTooltipLatexInsert);


    // Create the Action that will be called.
    EV_Menu_Action* myLatexAction = new EV_Menu_Action(
	FromLatexID,                     // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	1,                      // yes, we raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	"AbiMathView_LatexInsert",  // name of callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );


    pActionSet->addAction(myLatexAction);

   endEquationID= pFact->addNewMenuAfter("Main",NULL,AbiMathView_MenuLabelLatexInsert,EV_MLF_EndSubMenu);
   UT_DEBUGMSG(("End Equation ID %d \n",endEquationID));
    pFact->addNewLabel(NULL,endEquationID,AbiMathView_MenuEndEquation,NULL);

 
 // Create the Action that will be called.
    EV_Menu_Action* myEndEquationAction = new EV_Menu_Action(
	endEquationID,          // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	0,                      // no, we raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	NULL,                   // name of callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );


    pActionSet->addAction(myEndEquationAction);
    
    pApp->rebuildMenus();
}

static void
AbiMathView_removeFromMenus()
{
	// First we need to get a pointer to the application itself.
	XAP_App *pApp = XAP_App::getApp();

	// remove the edit method
	EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer() ;
	EV_EditMethod * pEM = ev_EditMethod_lookup("AbiMathView_FileInsert") ;
	pEMC->removeEditMethod(pEM) ;
	DELETEP(pEM) ;
	pEM = ev_EditMethod_lookup ("AbiMathView_LatexInsert") ;
	pEMC->removeEditMethod (pEM) ;
	DELETEP( pEM ) ;

	// now remove crap from the menus
	XAP_Menu_Factory * pFact = pApp->getMenuFactory();

	pFact->removeMenuItem("Main",NULL,newEquationID);
	pFact->removeMenuItem("Main",NULL,FromFileID);
	pFact->removeMenuItem("Main",NULL,FromLatexID);
	pFact->removeMenuItem("Main",NULL, endEquationID);

	pApp->rebuildMenus();
}
 

XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode)
{
	XAP_String_Id String_id;

	switch(errorCode)
	{
		case -301:
			String_id = AP_STRING_ID_MSG_IE_FileNotFound;
			break;

		case -302:
			String_id = AP_STRING_ID_MSG_IE_NoMemory;
			break;

		case -303:
			String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
			//AP_STRING_ID_MSG_IE_UnknownType;
			break;

		case -304:
			String_id = AP_STRING_ID_MSG_IE_BogusDocument;
			break;

		case -305:
			String_id = AP_STRING_ID_MSG_IE_CouldNotOpen;
			break;

		case -306:
			String_id = AP_STRING_ID_MSG_IE_CouldNotWrite;
			break;

		case -307:
			String_id = AP_STRING_ID_MSG_IE_FakeType;
			break;

		case -311:
			String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
			break;

		default:
			String_id = AP_STRING_ID_MSG_ImportError;
	}

	return pFrame->showMessageBox(String_id,
			XAP_Dialog_MessageBox::b_O,
			XAP_Dialog_MessageBox::a_OK,
			pNewFile);
}

static bool s_AskForMathMLPathname(XAP_Frame * pFrame,
					   char ** ppPathname)
{
	// raise the file-open dialog for inserting a MathML equation.
	// return a_OK or a_CANCEL depending on which button
	// the user hits.
	// return a pointer a g_strdup()'d string containing the
	// pathname the user entered -- ownership of this goes
	// to the caller (so free it when you're done with it).

	UT_DEBUGMSG(("s_AskForMathMLPathname: frame %p\n", pFrame));

	UT_return_val_if_fail (ppPathname, false);
	*ppPathname = NULL;

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_INSERTMATHML));
	UT_return_val_if_fail (pDialog, false);

	pDialog->setCurrentPathname(NULL);
	pDialog->setSuggestFilename(false);

	/* 
	TODO: add a "MathML (.mml)" entry to the file type list, and set is as the default file type

	pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));
	*/

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		const char * szResultPathname = pDialog->getPathname();
		UT_DEBUGMSG(("MATHML Path Name selected = %s \n",szResultPathname));
		if (szResultPathname && *szResultPathname)
			*ppPathname = g_strdup(szResultPathname);

		UT_sint32 type = pDialog->getFileType();

		// If the number is negative, it's a special type.
		// Some operating systems which depend solely on filename
		// suffixes to identify type (like Windows) will always
		// want auto-detection.
		if (type < 0)
		{
			switch (type)
			{
			case XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO:
				// do some automagical detecting
				break;
			default:
				// it returned a type we don't know how to handle
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
		else
		{
			/* todo */
		}
	}

	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}

//
// AbiMathView_FileInsert
// -------------------
//   This is the function that we actually call to insert the MathML.
//
bool 
AbiMathView_FileInsert(AV_View* /*v*/, EV_EditMethodCallData* /*d*/)
{
	// Get the current view that the user is in.
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());
	PD_Document * pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
	char* pNewFile = NULL;


	bool bOK = s_AskForMathMLPathname(pFrame,&pNewFile);
	
	if (!bOK || !pNewFile)
	{
		UT_DEBUGMSG(("ARRG! bOK = %d pNewFile = %s \n",bOK,pNewFile));
		return false;
	}
	UT_UTF8String sNewFile = pNewFile;

	// we own storage for pNewFile and must free it.
	FREEP(pNewFile);

	UT_DEBUGMSG(("fileInsertMathML: loading [%s]\n",sNewFile.utf8_str()));
   
	IE_Imp_MathML * pImpMathML = new IE_Imp_MathML(pDoc, pMathManager->EntityTable());
	UT_Error errorCode = pImpMathML->importFile(sNewFile.utf8_str());

	if (errorCode != UT_OK)
	{
		s_CouldNotLoadFileMessage(pFrame, sNewFile.utf8_str(), errorCode);
		DELETEP(pImpMathML);
		return false;
	}
	
	UT_UTF8String PbMathml = (const char*)((pImpMathML->getByteBuf())->getPointer(0));
	char *buf;
	int length;
	gboolean compact;

	if (go_mathml_to_itex (PbMathml.utf8_str(), &buf, &length, &compact, NULL))
	{
		UT_UTF8String Pbitex(buf, length);
		pView->cmdInsertLatexMath(Pbitex, PbMathml, compact);
		g_free(buf);
	}
	else
	{
		// Conversion of MathML to LaTeX fails
		// Inserting only the MathML

		/* Create the data item */
		UT_uint32 uid = pDoc->getUID(UT_UniqueId::Image);
		UT_UTF8String sUID;
		UT_UTF8String_sprintf(sUID,"%d",uid);
		pDoc->createDataItem(sUID.utf8_str(), false, pImpMathML->getByteBuf(), 
		                     "application/mathml+xml", NULL); 

		/* Insert the MathML Object */
		PT_DocPosition pos = pView->getPoint();
		pView->cmdInsertMathML(sUID.utf8_str(),pos); 
	}

	DELETEP(pImpMathML);
	return true;
}


//
// AbiMathView_LatexInsert
// -------------------
//   This is the function that we actually call to insert the MathML from
//   a Latex expression.
//
bool 
AbiMathView_LatexInsert(AV_View* v, EV_EditMethodCallData* /*d*/)
{
	FV_View * pView = static_cast<FV_View *>(v);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
	  = static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory());

	AP_Dialog_Latex * pDialog 
		= static_cast<AP_Dialog_Latex *>(pDialogFactory->requestDialog(AP_DIALOG_ID_LATEX));
	UT_return_val_if_fail(pDialog, false);

	if (pDialog->isRunning())
	{
		pDialog->activate();
	}
	else
	{
		pDialog->runModeless(pFrame);
	}

	return true;
}

 GR_AbiMathItems::GR_AbiMathItems(void):
  m_iAPI(0),
  m_bHasSnapshot(false)
{
}

 GR_AbiMathItems::~GR_AbiMathItems(void)
{
}


GR_LasemMathManager::GR_LasemMathManager(GR_Graphics* pG)
  : GR_EmbedManager(pG), 
    m_CurrentUID(-1),
    m_pDoc(NULL)
{
  m_vecLasemMathView.clear();
  m_vecItems.clear();
}

GR_LasemMathManager::~GR_LasemMathManager()
{ 
     UT_VECTOR_PURGEALL(GR_AbiMathItems *, m_vecItems);     
}

GR_EmbedManager * GR_LasemMathManager::create(GR_Graphics * pG)
{
  return static_cast<GR_EmbedManager *>(new GR_LasemMathManager(pG));
}

const char * GR_LasemMathManager::getObjectType(void) const
{
  return "mathml";
}

LasemMathView * GR_LasemMathManager::last_created_view = NULL;

void GR_LasemMathManager::initialize(void)
{
}

UT_sint32  GR_LasemMathManager::_makeLasemMathView()
{
     last_created_view = new LasemMathView(this);
     m_vecLasemMathView.addItem(last_created_view);
     return m_vecLasemMathView.getItemCount()-1;
}

void GR_LasemMathManager::_loadMathMl(UT_sint32 uid, UT_UTF8String& sMathBuf)
{
  LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
  UT_return_if_fail(pLasemMathView);
  UT_DEBUGMSG(("loading |%s| \n",sMathBuf.utf8_str()));
  pLasemMathView->loadBuffer(sMathBuf);
/*	if(sMathBuf)
	{
		UT_DEBUGMSG(("Attempt to load |%s| \n failed \n",sMathBuf.utf8_str()));

		UT_UTF8String sFailed = "<math xmlns='http://www.w3.org/1998/Math/MathML' display='inline'><merror><mtext>";
		sFailed += "failed"; // TODO: need a better message!
		sFailed += "</mtext></merror></math>";
		pLasemMathView->loadBuffer(sFailed);
	}
*/
}

UT_sint32 GR_LasemMathManager::makeEmbedView(AD_Document * pDoc, UT_uint32 api, G_GNUC_UNUSED const char * szDataID)
{
        if(m_pDoc == NULL)
        {
          m_pDoc = static_cast<PD_Document *>(pDoc);
        }
        else
        {
          UT_ASSERT(m_pDoc == static_cast<PD_Document *>(pDoc));
        }
        UT_sint32 iNew = _makeLasemMathView();
        GR_AbiMathItems * pItem = new GR_AbiMathItems();
        pItem->m_iAPI = api;
        pItem->m_bHasSnapshot = false;
        m_vecItems.addItem(pItem);
        UT_ASSERT(m_vecItems.getItemCount() == (iNew+1));
        return iNew;
}

void GR_LasemMathManager::makeSnapShot(UT_sint32 uid, G_GNUC_UNUSED UT_Rect & rec)
{
	GR_AbiMathItems * pItem = m_vecItems.getNthItem(uid);
	UT_return_if_fail(pItem);  
	LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
	const PP_AttrProp * pSpanAP = NULL;
	PT_AttrPropIndex api = pItem->m_iAPI;
	/* bool b = */ m_pDoc->getAttrProp(api, &pSpanAP);
	const char * pszDataID = NULL;
	pSpanAP->getAttribute("dataid", pszDataID);
	UT_ByteBuf *pBuf = NULL;
	std::string mime_type;
	if ((pBuf = pLasemMathView->getSnapShot()))
	  {
		UT_UTF8String sID = "snapshot-svg-";
		sID += pszDataID;
		if(pItem->m_bHasSnapshot)
		  {
			m_pDoc->replaceDataItem(sID.utf8_str(),reinterpret_cast< const UT_ByteBuf *>(pBuf));
		  }
		else
		  {
			m_pDoc->createDataItem(sID.utf8_str(),false,reinterpret_cast< const UT_ByteBuf *>(pBuf),mime_type,NULL);
			pItem->m_bHasSnapshot = true;
		  }
		delete pBuf;
	  }
}

void GR_LasemMathManager::initializeEmbedView(G_GNUC_UNUSED UT_sint32 uid)
{
}

void GR_LasemMathManager::loadEmbedData(UT_sint32 uid)
{
  LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
  UT_return_if_fail(pLasemMathView);
  const PP_AttrProp * pSpanAP = NULL;
  GR_AbiMathItems * pItem = m_vecItems.getNthItem(uid);
  UT_return_if_fail(pItem);  
  PT_AttrPropIndex api = pItem->m_iAPI;
  bool bHaveProp = m_pDoc->getAttrProp(api, &pSpanAP);
  UT_return_if_fail(bHaveProp);
  const char * pszDataID = NULL;
  bool bFoundDataID = pSpanAP->getAttribute("dataid", pszDataID);
  UT_UTF8String sMathML;
  if (bFoundDataID && pszDataID)
  {
       const UT_ByteBuf * pByteBuf = NULL;
       bFoundDataID = m_pDoc->getDataItemDataByName(pszDataID, 
						    const_cast<const UT_ByteBuf **>(&pByteBuf),
						    NULL, NULL);
       if (bFoundDataID)
       {
            UT_UCS4_mbtowc myWC;
            sMathML.appendBuf( *pByteBuf, myWC);
       }
  }
 UT_return_if_fail(bFoundDataID);
 UT_return_if_fail(pszDataID);
  UT_DEBUGMSG(("The Mathml string is... \n %s \n",sMathML.utf8_str()));
  _loadMathMl(uid, sMathML);
}

UT_sint32 GR_LasemMathManager::getWidth(G_GNUC_UNUSED UT_sint32 uid)
{
  LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
  UT_return_val_if_fail (pLasemMathView, 0);
  return pLasemMathView->getWidth();
}


UT_sint32 GR_LasemMathManager::getAscent(G_GNUC_UNUSED UT_sint32 uid)
{
  LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
  UT_return_val_if_fail (pLasemMathView, 0);
  return pLasemMathView->getAscent();
}


UT_sint32 GR_LasemMathManager::getDescent(G_GNUC_UNUSED UT_sint32 uid)
{
  LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
  UT_return_val_if_fail (pLasemMathView, 0);
  return pLasemMathView->getDescent();
}

void GR_LasemMathManager::setColor(G_GNUC_UNUSED UT_sint32 uid, G_GNUC_UNUSED const UT_RGBColor& c)
{
	LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
	return pLasemMathView->setColor (c);
}

bool GR_LasemMathManager::setFont(UT_sint32 uid, const GR_Font * pFont)
{
	LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
	pLasemMathView->setFont (pFont);
	return true;
}

void GR_LasemMathManager::setDisplayMode(UT_sint32 uid, AbiDisplayMode mode)
{
	LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
	pLasemMathView->setDisplayMode (mode);
}

void GR_LasemMathManager::render(UT_sint32 uid, UT_Rect & rec)
{
        LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
        UT_return_if_fail(pLasemMathView);
        pLasemMathView->render(rec);
}

void GR_LasemMathManager::releaseEmbedView(UT_sint32 uid)
{
       LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
       delete pLasemMathView;
       m_vecLasemMathView.setNthItem(uid,NULL,NULL); //NULL it out so we don't affect the other uid's
}

void GR_LasemMathManager::setRun(UT_sint32 uid, fp_Run *pRun)
{
	LasemMathView * pLasemMathView = m_vecLasemMathView.getNthItem(uid);
	pLasemMathView->SetRun (pRun);
}

void GR_LasemMathManager::updateData(UT_sint32 uid, UT_sint32 api)
{
	GR_AbiMathItems * pItem = m_vecItems.getNthItem(uid);
	UT_return_if_fail(pItem);  
	pItem->m_iAPI = api;
}

bool GR_LasemMathManager::isDefault()
{
        return false;
}

bool GR_LasemMathManager::isEdittable(G_GNUC_UNUSED UT_sint32 uid)
{
        return true;
}

bool GR_LasemMathManager::convert(G_GNUC_UNUSED UT_uint32 iConType, G_GNUC_UNUSED UT_ByteBuf & From, G_GNUC_UNUSED UT_ByteBuf & To)
{
  	XAP_App * pApp = XAP_App::getApp();
	XAP_Frame * pFrame = pApp->getLastFocussedFrame();

	if (iConType != 0 && iConType != 1)
	{
		return false;
	}

	/* add a pair of enclosing brackets \[ \] */
	UT_UTF8String sLatex;
	UT_UCS4_mbtowc myWC;
	sLatex += (iConType)? "$": "\\[";
	sLatex.appendBuf(From, myWC);
	sLatex += (iConType)? "$": "\\]";

	char * mathml = lsm_itex_to_mathml(sLatex.utf8_str(), sLatex.size());
	
	if (!mathml)
	{
		pFrame->showMessageBox("itex2MML failed to convert the LaTeX equation into MathML, sorry!\n", // TODO: fix message
			XAP_Dialog_MessageBox::b_O, 
			XAP_Dialog_MessageBox::a_OK);
		return false;
	}

	UT_UTF8String sMathML(mathml);
	lsm_itex_free_mathml_buffer(mathml);

	if (sMathML.size() == 0)
	{
	  UT_UTF8String sErrMessage = "itex2MML conversion from LaTex equation resulted in zero-length MathML!\n"; // TODO: fix message
		//sErrMessage += sLatex;
		sErrMessage += "\n";
		// sErrMessage += FullLine;
		pFrame->showMessageBox(sErrMessage.utf8_str(),
				XAP_Dialog_MessageBox::b_O,
				XAP_Dialog_MessageBox::a_OK);
		return false;
	}

	UT_DEBUGMSG(("Input MathML %s \n", sMathML.utf8_str()));

	return EntityTable().convert(sMathML.utf8_str(), sMathML.size(), To);
}

void GR_LasemMathManager::setDefaultFontSize(G_GNUC_UNUSED UT_sint32 uid, G_GNUC_UNUSED UT_sint32 iSize)
{

}

LasemMathView::LasemMathView(GR_LasemMathManager * pMathMan): m_pMathMan(pMathMan)
{
	width = height = 0;
        ascent = descent = 0;
        
        font = NULL;
        color = NULL;
        itex = NULL;
        
	view=NULL;
        mathml = lsm_dom_implementation_create_document(NULL, "math");

	m_Guru = NULL;
}

LasemMathView::~LasemMathView(void)
{
	if(mathml!=NULL)
		g_object_unref(mathml);		
	if(view!=NULL)
		g_object_unref(view);
	if (m_Guru)
		gtk_widget_destroy (m_Guru);
}

void LasemMathView::loadBuffer(UT_UTF8String & sMathml)
{
	g_object_unref(mathml);
	if (view != NULL)	
		g_object_unref(view);
	math_element = NULL;
	style_element = NULL;
        mathml = lsm_dom_document_new_from_memory(sMathml.utf8_str(),sMathml.length(),NULL);
	if (mathml == NULL)
		return;
	math_element = LSM_DOM_NODE (lsm_dom_document_get_document_element (mathml));
	style_element = LSM_DOM_NODE (lsm_dom_document_create_element (mathml, "mstyle"));
	LsmDomNode *child;			
	while ((child = lsm_dom_node_get_first_child(math_element)))
	{	
		lsm_dom_node_remove_child(math_element, child);
		lsm_dom_node_append_child(style_element, child);
	}
	lsm_dom_node_append_child(math_element, style_element);
	view = lsm_dom_document_create_view (mathml);
}

void LasemMathView::render(UT_Rect & rec)
{
	UT_return_if_fail (mathml);
	if((rec.width == 0) || (rec.height ==0))
	{
		return;
	}
	GR_CairoGraphics *pUGG = static_cast<GR_CairoGraphics*>(m_pMathMan->getGraphics());
	pUGG->beginPaint();
	cairo_t *cr = pUGG->getCairo();
	UT_sint32 _width = pUGG->tdu(rec.width * UT_LAYOUT_RESOLUTION);
	double zoom;
	if (mathml == NULL || height == 0 || width == 0)
		return;
	zoom = _width / width / 72.;
	cairo_save (cr);
	cairo_translate (cr, pUGG->tdu(rec.left), pUGG->tdu(rec.top - ascent));
	cairo_scale (cr,zoom, zoom);
	view = lsm_dom_document_create_view (mathml);
	lsm_dom_view_render (view, cr, 0., m_y * 72. / UT_LAYOUT_RESOLUTION);
	cairo_new_path (cr);
	cairo_restore (cr);
	pUGG->endPaint();
}

cairo_status_t abi_CairoWrite(UT_ByteBuf * buf, unsigned char * data, unsigned int length)
{
	return (buf->append (static_cast<UT_Byte*>(data), static_cast<UT_uint32>(length)))?
			CAIRO_STATUS_SUCCESS: CAIRO_STATUS_WRITE_ERROR;
}

UT_ByteBuf *LasemMathView::getSnapShot ()
{
	UT_return_val_if_fail (mathml, NULL);
	int _height = ascent + descent;
	if (_height == 0 || (int) width == 0)
		return NULL;
	size_t length;
	
	UT_ByteBuf *pBuf = new UT_ByteBuf ();
	cairo_surface_t *surface = cairo_svg_surface_create_for_stream (reinterpret_cast<cairo_write_func_t>(abi_CairoWrite),
										pBuf, width * 72./UT_LAYOUT_RESOLUTION, height * 72./UT_LAYOUT_RESOLUTION);
	cairo_t *cr = cairo_create (surface);
	cairo_surface_destroy (surface);
	lsm_dom_view_render (view, cr, 0., m_y * 72. / UT_LAYOUT_RESOLUTION);
	cairo_destroy (cr);
	length = (size_t)pBuf->getLength();
	
	const UT_Byte *buf = reinterpret_cast <const UT_Byte*> (pBuf);
	if (!buf || !length)
		return NULL;
	UT_ByteBuf *pSVGBuf = new UT_ByteBuf ();
	pSVGBuf->append (buf, (int)length);
	return pSVGBuf;
}

 void LasemMathView :: setFont(const GR_Font * pFont)
 {      
	UT_DEBUGMSG(("Entering SetFont..\n"));
        UT_return_if_fail(pFont);
	const GR_PangoFont *pPF = dynamic_cast<const GR_PangoFont *>(pFont);
	UT_return_if_fail(pPF);      
    if (pPF->getPangoDescription()!= NULL) 
    {
		LsmDomElement *_style_element;
		char *value;
		double _width, _height, _baseline;

		g_free (font);
		font = pango_font_description_to_string(pPF->getPangoDescription());
	
		_style_element = LSM_DOM_ELEMENT(style_element);
		if (pango_font_description_get_weight(pPF->getPangoDescription()) >= PANGO_WEIGHT_BOLD)
		{
			if (pango_font_description_get_style(pPF->getPangoDescription()) == PANGO_STYLE_NORMAL)
				lsm_dom_element_set_attribute(_style_element, "mathvariant", "bold");
			else
				lsm_dom_element_set_attribute(_style_element, "mathvariant", "bold-italic");
		}
		else
		{
			if (pango_font_description_get_style(pPF->getPangoDescription()) == PANGO_STYLE_NORMAL)
				lsm_dom_element_set_attribute(_style_element, "mathvariant", "normal");
			else
				lsm_dom_element_set_attribute(_style_element, "mathvariant", "italic");
		}

		lsm_dom_element_set_attribute(_style_element, "mathfamily",
						   pango_font_description_get_family(pPF->getPangoDescription()));

		value = g_strdup_printf("%gpt", pango_units_to_double (
				pango_font_description_get_size(pPF->getPangoDescription())));
		lsm_dom_element_set_attribute(_style_element, "mathsize", value);
		g_free(value);
		view = lsm_dom_document_create_view(mathml);
		lsm_dom_view_get_size(view, &_width, &_height, &_baseline);
		// lasem returns the ink measures, so we need to adjust the height
		PangoLayout *pl = view->measure_pango_layout;
		pango_layout_set_font_description(pl, (pPF->getPangoDescription()));
		pango_layout_set_text(pl, "lj", 2);
		PangoRectangle log, ink;
		pango_layout_get_extents(pl, &ink, &log);
        width = (UT_sint32) rint(_width / 72. * UT_LAYOUT_RESOLUTION);
        height = (UT_sint32) rint((_height + pango_units_to_double(log.height - ink.height)) / 72. * UT_LAYOUT_RESOLUTION);
 		m_y = (UT_sint32) rint(pango_units_to_double(ink.y) / 72. * UT_LAYOUT_RESOLUTION);
        ascent = (UT_sint32) rint((_baseline) / 72. * UT_LAYOUT_RESOLUTION) + m_y;
		descent = height - ascent;
	}
	UT_DEBUGMSG(("font : %s \n",font));	
 }

void LasemMathView::setColor(const UT_RGBColor& c)
{
	UT_DEBUGMSG(("Entering SetColor..\n"));
	UT_HashColor pHashColor;
	color = g_strdup(pHashColor.setColor(c));
	
	LsmDomElement *_style_element;
	_style_element = LSM_DOM_ELEMENT (style_element);
      
       lsm_dom_element_set_attribute (_style_element, "mathcolor", color);
	view = lsm_dom_document_create_view (mathml);
	UT_DEBUGMSG(("color : %s \n",color));
}

void LasemMathView::setDisplayMode(AbiDisplayMode mode)
{
	compact = mode == ABI_DISPLAY_INLINE;
	lsm_dom_element_set_attribute (LSM_DOM_ELEMENT (style_element), "displaystyle",
				       compact? "false" : "true");
}

ABI_PLUGIN_DECLARE(AbiMathView)

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------
  
ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	mi->name = "AbiMathView";
	mi->desc = "The plugin allows AbiWord to import MathML documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Martin Sevior <msevior@physics.unimelb.edu.au>";
	mi->usage = "No Usage";
    
	// Add to AbiWord's plugin listeners
	XAP_App * pApp = XAP_App::getApp();	
	pMathManager = new GR_LasemMathManager(NULL);
	pApp->registerEmbeddable(pMathManager);

	// Add to AbiWord's menus
	AbiMathView_addToMenus();
     
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

	XAP_App * pApp = XAP_App::getApp();
	pApp->unRegisterEmbeddable(pMathManager->getObjectType());
	DELETEP(pMathManager);
	AbiMathView_removeFromMenus();

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
	return 1; 
}
