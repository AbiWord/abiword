/*
 * Presentation
 * Copyright (C) 2007 by Martin Sevior
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

#define USE_PIXMAP 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string>

#include <glib.h>

#include "xap_Module.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "ev_EditMethod.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "ie_types.h"
#include "ap_Convert.h"
#include "ap_EditMethods.h"
#include "ev_EditBits.h"
#include "Presentation.h"
#include "ap_LoadBindings.h"
#include "ev_NamedVirtualKey.h"
#include "ut_bytebuf.h"
#include "ap_Menu_Id.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "xap_Menu_Layouts.h"
#include "gr_Graphics.h"
#include "gr_DrawArgs.h"
#include "fp_Page.h"
#include "gr_Painter.h"
#if USE_PIXMAP
#include <gdk/gdk.h>
#include "gr_UnixPangoGraphics.h"
#include "gr_UnixPangoPixmapGraphics.h"
#endif

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_presentation_register
#define abi_plugin_unregister abipgn_presentation_unregister
#define abi_plugin_supports_version abipgn_presentation_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("Presentation")
#endif

#define RES_TO_STATUS(a) ((a) ? 0 : -1)

static bool Presentation_start (AV_View * v, EV_EditMethodCallData * d);
static bool Presentation_end (AV_View * v, EV_EditMethodCallData * d);
static bool Presentation_nextPage (AV_View * v, EV_EditMethodCallData * d);
static bool Presentation_prevPage (AV_View * v, EV_EditMethodCallData * d);
static bool Presentation_context (AV_View * v, EV_EditMethodCallData * d);

Presentation myPresentation;
EV_EditMouseContext PresentationContextID =  EV_EMC_EMBED;
static XAP_Menu_Id presentationID;
static const char * szPresentation = "Presentation";
static const char * szPresentationStatus = "View the document in presentation mode";
static const char * szNextSlide = "Next Slide";
static const char * szPrevSlide = "Previous Slide";
static const char * szEndPresentation = "End Presentation";
static XAP_Menu_Id nextSlideID;
static XAP_Menu_Id prevSlideID;
static XAP_Menu_Id endPresentationID;
//
// Presentation_registerMethod()
// -----------------------
//   Adds Presentation_start, Presentation_end to the EditMethod list
//
static void
Presentation_registerMethod ()
{
	// First we need to get a pointer to the application itself.
	XAP_App *pApp = XAP_App::getApp ();

	// Create an EditMethod that will link our method's name with
	// it's callback function.  This is used to link the name to 
	// the callback.
	EV_EditMethod *myEditMethod = new EV_EditMethod ("Presentation_start",	// name of callback function
							 Presentation_start,	// callback function itself.
							 0,	// no additional data required.
							 ""	// description -- allegedly never used for anything
							 );

	// Now we need to get the EditMethod container for the application.
	// This holds a series of Edit Methods and links names to callbacks.
	EV_EditMethodContainer *pEMC = pApp->getEditMethodContainer ();

	// We have to add our EditMethod to the application's EditMethodList
	// so that the application will know what callback to call when a call

	pEMC->addEditMethod (myEditMethod);

	myEditMethod = new EV_EditMethod ("Presentation_end",	// name of callback function
							 Presentation_end,	// callback function itself.
							 0,	// no additional data required.
							 ""	// description -- allegedly never used for anything
							 );

	pEMC->addEditMethod (myEditMethod);

	myEditMethod = new EV_EditMethod ("Presentation_nextPage",	// name of callback function
							 Presentation_nextPage,	// callback function itself.
							 0,	// no additional data required.
							 ""	// description -- allegedly never used for anything
							 );

	pEMC->addEditMethod (myEditMethod);

	myEditMethod = new EV_EditMethod ("Presentation_prevPage",	// name of callback function
							 Presentation_prevPage,	// callback function itself.
							 0,	// no additional data required.
							 ""	// description -- allegedly never used for anything
							 );

	pEMC->addEditMethod (myEditMethod);

	myEditMethod = new EV_EditMethod ("Presentation_context",	// name of callback function
							 Presentation_context,	// callback function itself.
							 0,	// no additional data required.
							 ""	// description -- allegedly never used for anything
							 );

	pEMC->addEditMethod (myEditMethod);

    // Now we need to grab an ActionSet.  This is going to be used later
    // on in our for loop.  Take a look near the bottom.
    EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();

    XAP_Menu_Factory * pFact = pApp->getMenuFactory();

// Put it after 

    presentationID= pFact->addNewMenuAfter("Main",NULL,AP_MENU_ID_VIEW_SHOWPARA,EV_MLF_Normal);
    UT_DEBUGMSG(("presentationID %d \n",presentationID));


    pFact->addNewLabel(NULL,presentationID,szPresentation,szPresentationStatus);

    // Create the Action that will be called.
    EV_Menu_Action* myPresentationAction = new EV_Menu_Action(
	presentationID,          // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	0,                      // no, we don't raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	"Presentation_start",   //  callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );

    // Now what we need to do is add this particular action to the ActionSet
    // of the application.  This forms the link between our new ID that we 
    // got for this particular frame with the EditMethod that knows how to 
    // call our callback function.  

    pActionSet->addAction(myPresentationAction);


    //
    // OK Now build a context menu
    //
    PresentationContextID = pFact->createContextMenu("PresentationContext");
    prevSlideID = pFact->addNewMenuBefore("PresentationContext",NULL,0,EV_MLF_Normal,0);
    pFact->addNewLabel(NULL,prevSlideID,szPrevSlide, NULL);
    nextSlideID = pFact->addNewMenuBefore("PresentationContext",NULL,0,EV_MLF_Normal,0);
    pFact->addNewLabel(NULL,nextSlideID,szNextSlide, NULL);
    endPresentationID = pFact->addNewMenuBefore("PresentationContext",NULL,0,EV_MLF_Normal,0);
    pFact->addNewLabel(NULL,endPresentationID,szEndPresentation, NULL);

    myPresentationAction = new EV_Menu_Action(
	prevSlideID,          // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	0,                      // no, we don't raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	"Presentation_prevPage",   //  callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );
    pActionSet->addAction(myPresentationAction);
    myPresentationAction = new EV_Menu_Action(
	nextSlideID,          // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	0,                      // no, we don't raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	"Presentation_nextPage",   //  callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );
    pActionSet->addAction(myPresentationAction);
    myPresentationAction = new EV_Menu_Action(
	endPresentationID,          // id that the layout said we could use
	0,                      // no, we don't have a sub menu.
	0,                      // no, we don't raise a dialog.
	0,                      // no, we don't have a checkbox.
	0,                      // no radio buttons for me, thank you
	"Presentation_end",   //  callback function to call.
	NULL,                   // don't know/care what this is for
	NULL                    // don't know/care what this is for
        );
    pActionSet->addAction(myPresentationAction);


}

static void
Presentation_RemoveFromMethods ()
{
	// First we need to get a pointer to the application itself.
	XAP_App *pApp = XAP_App::getApp ();

	// remove the edit method
	EV_EditMethodContainer *pEMC = pApp->getEditMethodContainer ();
	EV_EditMethod *pEM = ev_EditMethod_lookup ("Presentation_start");

	pEMC->removeEditMethod (pEM);
	DELETEP (pEM);
	
	pEM = ev_EditMethod_lookup ("Presentation_end");
	pEMC->removeEditMethod (pEM);
	DELETEP (pEM);
	
	pEM = ev_EditMethod_lookup ("Presentation_nextPage");
	pEMC->removeEditMethod (pEM);
	DELETEP (pEM);
	
	pEM = ev_EditMethod_lookup ("Presentation_prevPage");
	pEMC->removeEditMethod (pEM);
	DELETEP (pEM);
	
	pEM = ev_EditMethod_lookup ("Presentation_context");
	pEMC->removeEditMethod (pEM);
	DELETEP (pEM);

	XAP_Menu_Factory * pFact = pApp->getMenuFactory();

	pFact->removeMenuItem("Main",NULL,presentationID);
}

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------

ABI_BUILTIN_FAR_CALL int
abi_plugin_register (XAP_ModuleInfo * mi)
{
	mi->name = "Presentation";
	mi->desc = "This enables AbiWord to make presentations";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Martin Sevior <msevior@physics.unimelb.edu.au>";
	mi->usage = "Presentaton_start";

	Presentation_registerMethod ();
	return 1;
}

ABI_BUILTIN_FAR_CALL int
abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = nullptr;
	mi->desc = nullptr;
	mi->version = nullptr;
	mi->author = nullptr;
	mi->usage = nullptr;

	Presentation_RemoveFromMethods ();

	return 1;
}

ABI_BUILTIN_FAR_CALL int
abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
	return 1;
}

// -----------------------------------------------------------------------
//
//     Presentation_start Invocation Code
//
// -----------------------------------------------------------------------

//
//  Presentation_start
// -------------------
//   This is the function that puts AbiWord into full screen mode and set the 
//   keybindings for a presentation.
//
static bool
Presentation_start (AV_View * v, EV_EditMethodCallData * /*d*/)
{
  myPresentation.start(v);
	
	return true;
}
//
// Presentation_end
// -------------------
// This functions takes AbiWord out of full screen mode and restores the 
// previous keybindings.
//
static bool
Presentation_end (AV_View * /*v*/, EV_EditMethodCallData * /*d*/)
{
  myPresentation.end();
  return true;
}

static bool
Presentation_nextPage (AV_View * /*v*/, EV_EditMethodCallData * /*d*/)
{
  myPresentation.showNextPage();
  return true;
}


static bool
Presentation_prevPage (AV_View * /*v*/, EV_EditMethodCallData * /*d*/)
{
  myPresentation.showPrevPage();
  return true;
}

static bool
Presentation_context (AV_View * v, EV_EditMethodCallData * d)
{
        FV_View * pView = static_cast<FV_View *>(v);
	XAP_Frame * pFrame = static_cast<XAP_Frame *> (pView->getParentData());
	UT_sint32 xPos = d->m_xPos;
	UT_sint32 yPos = d->m_yPos;
	const char * szContextMenuName =  XAP_App::getApp()->getMenuFactory()->FindContextMenu(PresentationContextID);
	UT_DEBUGMSG(("Context Menu Name is........ %s \n",szContextMenuName));
	if (!szContextMenuName)
		return false;
	bool res = pFrame->runModalContextMenu(pView,szContextMenuName,
									   xPos,yPos);
	pFrame->nullUpdate();
	GR_Graphics * pG  = pView->getGraphics();
	if(pG)
	  pG->allCarets()->disable();
	return res;
}


// Hack into seperate strings because C doesn't allow multi-line strings

static UT_String sPresBindings[] = {"name, Presentation\n",
			       "mse,B1,CU,Presentation_nextPage,,,,,,\n",
			       "mse,B1,CL,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CT,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CM,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CR,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CI,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CZ,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CF,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CH,Presentation_nextPage,,,,,,\n", 
			       "mse,B1,CV,Presentation_nextPage,,,,,,\n", 
			       "mse,B3,CU,Presentation_context,,,,,,\n",
			       "mse,B3,CL,Presentation_context,,,,,,\n", 
			       "mse,B3,CT,Presentation_context,,,,,,\n", 
			       "mse,B3,CM,Presentation_context,,,,,,\n", 
			       "mse,B3,CR,Presentation_context,,,,,,\n", 
			       "mse,B3,CI,Presentation_context,,,,,,\n", 
			       "mse,B3,CZ,Presentation_context,,,,,,\n", 
			       "mse,B3,CF,Presentation_context,,,,,,\n", 
			       "mse,B3,CH,Presentation_context,,,,,,\n", 
			       "mse,B3,CV,Presentation_context,,,,,,\n", 
			       "nvk,ESCAPE,Presentation_end,Presentation_end,Presentation_end,Presentation_end,,,,,\n",
			       "nvk,PAGEUP,Presentation_prevPage,,,,,,\n",
			       "nvk,PAGEDOWN,Presentation_nextPage,,,,,,\n",
			       "nvk,LEFT,Presentation_prevPage,,,,,,\n",
			       "nvk,UP,Presentation_prevPage,,,,,,\n",
			       "nvk,RIGHT,Presentation_nextPage,,,,,,\n",
			       "nvk,DOWN,Presentation_nextPage,,,,,,\n",
			       "nvk,F1,,Presentation_end,Presentation_end,Presentation_end,Presentation_end,,,,,\n",
			       "nvk,F2,,Presentation_end,Presentation_end,Presentation_end,Presentation_end,,,,,\n",
			       "nvk,F3,,Presentation_end,Presentation_end,Presentation_end,Presentation_end,,,,,\n",
			       "nvk,F4,,Presentation_end,Presentation_end,Presentation_end,Presentation_end,,,,,\n"
			       "nvk,F5,,Presentation_end,Presentation_end,Presentation_end,Presentation_end,,,,,\n",
			       ""};

Presentation::Presentation(void):
  m_pApp(NULL),
  m_pView(NULL),
  m_iPage(0),
  m_bDrewNext(false),
  m_bDrewPrev(false)
{
  m_pApp = XAP_App::getApp();
}


Presentation::~Presentation (void)
{

}

bool Presentation::_loadPresentationBindings(AV_View * pView)
{
  EV_EditMethodContainer * pEMC = m_pApp->getEditMethodContainer();
  g_return_val_if_fail(pEMC != nullptr, FALSE);
  if(m_pApp->getBindingMap("Presentation") != NULL)
  {
      return true;
  }
  // get the path where our app data is located
  XAP_App * pApp = static_cast<XAP_App*>(XAP_App::getApp());
  UT_String data_path( pApp->getAbiSuiteLibDir() );
  data_path += G_DIR_SEPARATOR;
  data_path += "Presentation.xml";

  EV_EditMethod * pLoadB = pEMC->findEditMethodByName ("com.abisource.abiword.loadbindings.fromURI");
  g_return_val_if_fail(pLoadB != nullptr, false);
  EV_EditMethodCallData calldata(data_path.c_str(),data_path.size());
  calldata.m_xPos = 0;
  calldata.m_yPos = 0;
  return (pLoadB->Fn(pView,&calldata) ? TRUE : FALSE);
}

bool Presentation::start(AV_View * view)
{
  EV_EditMethodContainer * pEMC = m_pApp->getEditMethodContainer();
  g_return_val_if_fail(pEMC != nullptr, FALSE);
  m_pView = static_cast<FV_View *>(view);
  m_sPrevBindings = m_pApp->getInputMode();
  _loadPresentationBindings(view);
  UT_sint32 i = m_pApp->setInputMode("Presentation");
  if(i < 0 )
    return false;
  // get a handle to the actual EditMethod
  EV_EditMethod * pFullScreen = pEMC->findEditMethodByName ("viewFullScreen");
  g_return_val_if_fail(pFullScreen != nullptr, false);
  const char * sz ="";
  EV_EditMethodCallData calldata(sz,0);
  calldata.m_xPos = 0;
  calldata.m_yPos = 0;
  m_iPage = 0;
  XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
  m_OldZoomType = pFrame->getZoomType();
  m_iOldZoom = pFrame->getZoomPercentage();

  pFrame->hideMenuScroll(true);
  bool b = false;
  b = (pFullScreen->Fn(view,&calldata) ? TRUE : FALSE);
  GR_Graphics * pG = m_pView->getGraphics();

  //
  // Let all the configure events propagate to their full extent
  //
  for(i= 0; i<20;i++)
    pFrame->nullUpdate();
  pFrame->setZoomType(XAP_Frame::z_PAGEWIDTH);
  i = m_pView-> calculateZoomPercentForPageWidth();
  pFrame->quickZoom(i);
  for(i= 0; i<20;i++)
    pFrame->nullUpdate();

  b= showNextPage();
  if(pG)
    pG->allCarets()->disable();
   return b;
}

bool Presentation::end(void)
{
  if(m_sPrevBindings.size() == 0)
    return false;
  EV_EditMethodContainer * pEMC = m_pApp->getEditMethodContainer();
  g_return_val_if_fail(pEMC != nullptr, FALSE);
  UT_sint32 i = m_pApp->setInputMode(m_sPrevBindings.c_str());
  if(i <=0 )
    return false;

  // get a handle to the actual EditMethod

  EV_EditMethod * pFullScreen = pEMC->findEditMethodByName ("viewFullScreen");
  g_return_val_if_fail(pFullScreen != nullptr, false);
  const char * sz ="";
  EV_EditMethodCallData calldata(sz,0);
  calldata.m_xPos = 0;
  calldata.m_yPos = 0;
  XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
  pFrame->hideMenuScroll(false);
  bool b= (pFullScreen->Fn(static_cast<AV_View *>(m_pView),&calldata) ? TRUE : FALSE);
  pFrame->setZoomType(m_OldZoomType);
  pFrame->setZoomPercentage(m_iOldZoom);
  pFrame->quickZoom(m_iOldZoom);
  return b;
}


bool Presentation::drawNthPage(UT_sint32 iPage)
{
  GR_Graphics * pG = m_pView->getGraphics();
  GR_Painter p(pG);
#if USE_PIXMAP
  GR_Image * pImage = renderPageToImage(iPage,pG->getZoomPercentage());
  UT_DEBUGMSG(("Image Display Width %d Image Display Height %d Zoom %d \n",pImage->getDisplayWidth(),pImage->getDisplayHeight(),pG->getZoomPercentage()));
  p.drawImage(pImage,0,0);
  delete pImage;
#else
  dg_DrawArgs da;
  da.pG = pG;
  da.xoff = 0;
  da.yoff = 0;
  m_pView->drawPage(iPage, &da);
  fp_Page * pPage = m_pView->getLayout()->getNthPage(iPage);
  UT_sint32 iTotalHeight = (pPage->getHeight() + m_pView->getPageViewSep())*iPage;
  m_pView->setYScrollOffset(iTotalHeight);
  if(pG)
    pG->allCarets()->disable();
#endif
  return true;
}
#if USE_PIXMAP
/*!
 * Caller must delete the returned image.
 */
GR_Image * Presentation::renderPageToImage(UT_sint32 iPage, UT_uint32 iZoom)
{
  GR_UnixPangoGraphics  * pVG = static_cast<GR_UnixPangoGraphics *>(m_pView->getGraphics());
  UT_sint32 iWidth = pVG->tdu(m_pView->getWindowWidth());
  UT_sint32 iHeight = pVG->tdu(m_pView->getWindowHeight());
	double ZoomRat = static_cast<double>(iZoom)/static_cast<double>(pVG->getZoomPercentage());
	iWidth = static_cast<UT_sint32>(static_cast<double>(iWidth)*ZoomRat);
	iHeight = static_cast<UT_sint32>(static_cast<double>(iHeight)*ZoomRat);
	//
	// Create an offscreen Graphics to draw into
	//
	UT_DEBUGMSG(("rederpagetoimage Width %d Height %d zoom %d \n",iWidth,iHeight,iZoom));
	GdkPixmap*  pPixmap = gdk_pixmap_new(pVG->getWindow(),iWidth,iHeight,-1);

	GR_UnixPixmapAllocInfo ai(pPixmap);

	GR_UnixPangoPixmapGraphics * pG = (GR_UnixPangoPixmapGraphics*)GR_UnixPangoPixmapGraphics::graphicsAllocator(ai);
	pG->setZoomPercentage(iZoom);
	GR_Painter * pPaint = new GR_Painter(pG);
	pPaint->clearArea(0,0,m_pView->getWindowWidth(),m_pView->getWindowHeight());
	dg_DrawArgs da;
	da.pG = pG;
	da.xoff = 0;
	da.yoff = 0;
	m_pView->getLayout()->setQuickPrint(pG);
	m_pView->drawPage(iPage, &da);
	UT_Rect r;
	r.left = 0;
	r.top = 0;
	r.width = pG->tlu(iWidth);
	r.height = pG->tlu(iHeight);
	GR_Image * pImage = pPaint->genImageFromRectangle(r);
	m_pView->getLayout()->setQuickPrint(NULL);
	DELETEP(pPaint);
	DELETEP(pG);
	return pImage;
}

#endif
bool  Presentation::showNext(void)
{
  return showNextPage();
}


bool  Presentation::showNextPage(void)
{
  if(m_bDrewPrev && (m_iPage + 1< static_cast<UT_sint32>(m_pView->getLayout()->countPages())))
  { 
      m_iPage++;
  }
  drawNthPage(m_iPage);
  if(m_iPage+1 < static_cast<UT_sint32>(m_pView->getLayout()->countPages()))
      m_iPage++;
  m_bDrewNext = true;
  m_bDrewPrev = false;
  return true;
}

bool  Presentation::showPrevPage(void)
{
  if(m_iPage > 0)
  {
    if(m_iPage > 1 && m_bDrewNext && (m_iPage +1 < m_pView->getLayout()->countPages()))
	  m_iPage--;
      m_iPage--;
  }
  else
    return false;
  drawNthPage(m_iPage);
  m_bDrewNext = false;
  m_bDrewPrev = true;
  return true;
}
