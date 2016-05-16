/*
 * 
 * Copyright (C) 2003 by Dom Lachowicz
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
#define abi_plugin_register abipgn_ots_register
#define abi_plugin_unregister abipgn_ots_unregister
#define abi_plugin_supports_version abipgn_ots_supports_version
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "ut_string_class.h"
#include "fl_SelectionPreserver.h"
#include "xap_EncodingManager.h"
#include "ie_types.h"
#include "ut_growbuf.h"

#include <ots/libots.h>

#ifdef TOOLKIT_GTK_ALL
#include <gtk/gtk.h>
#include "xap_UnixApp.h"
#include "xap_UnixDialogHelper.h"
#endif

static const char* Ots_MenuLabel = "&Summarize";
static const char* Ots_MenuTooltip = "Summarize your document or selected text";
#ifdef TOOLKIT_GTK_ALL

static int getSummaryPercent(void)
{
  // load the dialog from the UI file
  GtkBuilder* builder = newDialogBuilder("ots.ui");
  
  GtkWidget * window = GTK_WIDGET(gtk_builder_get_object(builder, "otsDlg"));
  GtkWidget * spin = GTK_WIDGET(gtk_builder_get_object(builder, "summarySpin"));

  abiRunModalDialog (GTK_DIALOG(window), XAP_App::getApp()->getLastFocussedFrame () , 
		     NULL, GTK_RESPONSE_CLOSE, false);

  int value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spin));
  fprintf (stderr, "DOM: percentage is %d\n", value);
  abiDestroyWidget (window);
  g_object_unref(G_OBJECT(builder));

  return value;
}

#else

static int getSummaryPercent(void)
{
  /* TODO: dialog to get the data */
  return 20;
}

#endif

bool 
AbiOts_invoke(AV_View* /*v*/, EV_EditMethodCallData * /*d*/)
{
  XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
  FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());

  const char * dictionary_file = XAP_EncodingManager::get_instance()->getLanguageISOName();
  OtsArticle * article = ots_new_article ();
  if (!ots_load_xml_dictionary (article, (const unsigned char *)dictionary_file))
    {
      ots_free_article (article);
      pFrame->showMessageBox("Ots: could not load dictionary!", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
      return false;
    }

  // todo: better way of extracting the text from the doc
  // martin suggests looking at fl_BlockLayout.cpp:GetAllText()
  
  UT_GrowBuf docTxt;
  pView->getTextInDocument(docTxt);

  if (!docTxt.getLength())
    return false;

  UT_UTF8String txt ((const UT_UCS4Char *)docTxt.getPointer(0), docTxt.getLength());

  // no need to keep this all around in memory
  docTxt.truncate(0);

  if (!txt.size()) {
    // nothing selected, so nothing to summarize
    ots_free_article (article);
    return false;
  }

  ots_parse_stream(reinterpret_cast<const unsigned char *>(txt.utf8_str()), txt.byteLength(), article);
  //ots_remove_dict_words(article);
  ots_grade_doc (article);
  ots_highlight_doc (article, getSummaryPercent());

  size_t summary_len = 0;
  unsigned char * utf8_summary = ots_get_doc_text (article, &summary_len);
  UT_UCS4String ucs4_summary (reinterpret_cast<const char *>(utf8_summary), summary_len);
  g_free (utf8_summary);

  if (ucs4_summary.size()) {
    XAP_Frame * newFrame = XAP_App::getApp()->newFrame();
    newFrame->loadDocument((const char*)NULL, IEFT_Unknown);
    newFrame->raise ();
    
    FV_View * newView = static_cast<FV_View*>(newFrame->getCurrentView());
    newView->cmdCharInsert(ucs4_summary.ucs4_str(), ucs4_summary.size());
    newView->moveInsPtTo (FV_DOCPOS_BOD);
  }

  ots_free_article (article);

  return true;
}

static void
Ots_removeFromMenus()
{
  XAP_App *pApp = XAP_App::getApp();

  EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer() ;
  EV_EditMethod * pEM = ev_EditMethod_lookup ( "AbiOts_invoke" ) ;
  pEMC->removeEditMethod ( pEM ) ;
  DELETEP( pEM ) ;

  int frameCount = pApp->getFrameCount();
  XAP_Menu_Factory * pFact = pApp->getMenuFactory();

  pFact->removeMenuItem("Main",NULL,Ots_MenuLabel);
  pFact->removeMenuItem("contextText",NULL,Ots_MenuLabel);
  for(int i = 0;i < frameCount;++i)
    {
      XAP_Frame* pFrame = pApp->getFrame(i);
      pFrame->rebuildMenus();
    }
}

static void
Ots_addToMenus()
{
  XAP_App *pApp = XAP_App::getApp();
    
  EV_EditMethod *myEditMethod = new EV_EditMethod("AbiOts_invoke",
						  AbiOts_invoke,
						  0,
						  "");
  
  EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();
  
  pEMC->addEditMethod(myEditMethod);

  EV_Menu_ActionSet* pActionSet = pApp->getMenuActionSet();
  
  int frameCount = pApp->getFrameCount();
  XAP_Menu_Factory * pFact = pApp->getMenuFactory();

  XAP_Menu_Id newID = pFact->addNewMenuAfter("contextText",NULL,"Bullets and &Numbering",EV_MLF_Normal);
  pFact->addNewLabel(NULL,newID,Ots_MenuLabel, Ots_MenuTooltip);

  pFact->addNewMenuAfter("Main",NULL,"&Word Count",EV_MLF_Normal,newID);
  
  EV_Menu_Action* myAction = new EV_Menu_Action(newID,                     // id that the layout said we could use
						0,                      // no, we don't have a sub menu.
#ifdef TOOLKIT_GTK_ALL
						1,                      // yes, we raise a dialog.
#else
						0,                      // no dialog
#endif
						0,                      // no, we don't have a checkbox.
						0,                      // not a radio button
						"AbiOts_invoke",  // name of callback function to call.
						NULL,                   // don't know/care what this is for
						NULL                    // don't know/care what this is for
						);
  
  pActionSet->addAction(myAction);
  
  for(int i = 0;i < frameCount;++i)
    {
      XAP_Frame* pFrame = pApp->getFrame(i);
      pFrame->rebuildMenus();
    }
}

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------

ABI_PLUGIN_DECLARE	("AbiOts")

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
    mi->name = "Ots plugin";
    mi->desc = "Open Text Summarizer for AbiWord";
    mi->version = ABI_VERSION_STRING;
    mi->author = "Dom Lachowicz";
    mi->usage = "No Usage";
    
    Ots_addToMenus();
    
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

    Ots_removeFromMenus() ;

    return 1;
}


ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
    return 1; 
}
