/*
 * AbiDash - Abiword framework for the notification based notification based
 * plugins. Designed in particular as a framework for Dashboard.
 * Copyright (C) 2004 by Martin Sevior
 * Copyright (C) 2004 QinetiQ Plc.
 *       Author Julian Satchell <j.satchell@eris.qinetiq.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Module.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "fv_View.h"
#include "xav_View.h"
#include "xav_Listener.h"
#include "fl_BlockLayout.h"
#include "pd_Document.h"

#include "dashboard-frontend.c"

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"

#include "ut_sleep.h"
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

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_dash_register
#define abi_plugin_unregister abipgn_dash_unregister
#define abi_plugin_supports_version abipgn_dash_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("Dash")
#endif

// -----------------------------------------------------------------------
//
//     AbiDash code
//
// -----------------------------------------------------------------------

class AbiDash : public AV_Listener
{
public:
AbiDash(XAP_App * pApp):
	m_pApp(pApp),
	m_pCurView(NULL),
	m_pCurDoc(NULL),
	m_pCurBlock(NULL),
	m_iCurPoint(0)
		{
		}

virtual ~AbiDash(void)
		{
		}
void setID(AV_ListenerId id)
		{
			m_lid = id;
		}

virtual  AV_ListenerType getType(void) { return AV_LISTENER_PLUGIN;}

AV_ListenerId	getID(void)
		{
			return m_lid;
		}
virtual bool notify(AV_View * pAView, const AV_ChangeMask mask)
		{
			PD_Document *    arg_pCurDoc;
			fl_BlockLayout * arg_pCurBlock;
			char *cluepkt, context[16], *time_txt=NULL;
			UT_UTF8String aText = "",bText="",cText="",sText="";
			const char *title_txt = NULL;
			const char *para_txt = NULL;
			const char *creator_txt = NULL;
			const char *coverage_txt = NULL;
			struct tm *tbork;
			time_t doc_time;
			//printf("I've been notified!! View = %x hint mask %d \n",pAView,mask);
			FV_View * pView = static_cast<FV_View *>(pAView);

			m_pCurView = pView;
			arg_pCurDoc = pView->getDocument();
			if (arg_pCurDoc != m_pCurDoc)
			{
				m_pCurDoc = arg_pCurDoc;
			}
			/*last opened time*/
			doc_time = m_pCurDoc->getLastOpenedTime() ;
			tbork = gmtime(&doc_time) ;
			time_txt = g_strdup_printf("%04d-%02d-%02d",
						   tbork->tm_year+1900,
						   tbork->tm_mon+1,
						   tbork->tm_mday);
			/* Gather various metadat items (if set))
			   /*title*/
			m_pCurDoc->getMetaDataProp (PD_META_KEY_TITLE, aText);
			if (aText.byteLength()>0) {
				title_txt =  aText.utf8_str();
			}
			/*creator*/
			m_pCurDoc->getMetaDataProp (PD_META_KEY_CREATOR , bText);
			if (bText.byteLength()>0) {
				creator_txt =  bText.utf8_str();
			}
			/*coverage*/
			m_pCurDoc->getMetaDataProp (PD_META_KEY_COVERAGE , cText);
			if (cText.byteLength()>0) {
				coverage_txt =  cText.utf8_str();
			}
			
			/*Use address of CurDoc as context id*/
			g_snprintf(context,16,"%p",m_pCurDoc);
			m_iCurPoint = pView->getPoint();

			arg_pCurBlock=	pView->getCurrentBlock();
			if (m_pCurBlock != arg_pCurBlock )
			{
				m_pCurBlock = arg_pCurBlock;
				if (m_pCurBlock!=NULL)
				{
					/*Now get block text*/
					m_pCurBlock->appendUTF8String(sText);
					if (sText.byteLength()>0) {
						para_txt =  sText.utf8_str();
					}
				}		
			}
			cluepkt = dashboard_build_cluepacket_then_free_clues 
				("Abiword",
				 true,
				 context, 
				 dashboard_build_clue (title_txt, "textblock", 10),
				 dashboard_build_clue (time_txt, "date", 10),
				 dashboard_build_clue (para_txt, "textblock", 10),
				 dashboard_build_clue (creator_txt, "name", 10),
				 dashboard_build_clue (coverage_txt, "latlong", 10),
				 NULL);
			dashboard_send_raw_cluepacket(cluepkt);
			//printf("%s\n",cluepkt);
			g_free(time_txt);
			g_free(cluepkt);
			return true;
		}

private:
	XAP_App *        m_pApp;
	FV_View *        m_pCurView;
	PD_Document *    m_pCurDoc;
	fl_BlockLayout * m_pCurBlock;
	PT_DocPosition   m_iCurPoint;
	AV_ListenerId    m_lid;
};


static AbiDash * pAbiDash = NULL;

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface 
//
// -----------------------------------------------------------------------

  
ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
    mi->name = "AbiDash";
    mi->desc = "The plugin allows AbiWord to send notifications to Dashboard";
    mi->version = ABI_VERSION_STRING;
    mi->author = "Martin Sevior <msevior@physics.unimelb.edu.au>";
    mi->usage = "No Usage";
    
    // Add to AbiWord's plugin listeners
	XAP_App * pApp = XAP_App::getApp();
	pAbiDash = new AbiDash(pApp);
	AV_ListenerId listenerID = 0;
	pApp->addListener(pAbiDash, &listenerID);
	pAbiDash->setID(listenerID);
	UT_DEBUGMSG(("Class AbiDash %x created! Listener Id %d \n",pAbiDash,listenerID));
    
    return 1;
}


ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
    mi->name = 0;
    mi->desc = 0;
    mi->version = 0;
    mi->author = 0;
    mi->usage = 0;

	XAP_App * pApp = XAP_App::getApp();
	UT_DEBUGMSG(("Removing AbiDash %x ! Listener Id %d \n",pAbiDash,pAbiDash->getID()));
	pApp->removeListener(pAbiDash->getID());

    return 1;
}


ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, UT_uint32 release)
{
    return 1; 
}
