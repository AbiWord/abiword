/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2005 INdT
 * Author: Renato Araujo <renato.filho@indt.org.br>
 *     
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


#include <stdio.h>

#include "ev_EditMethod.h"
#include "xap_UnixHildonApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xad_Document.h"

// something wrong with these headers -- they need to be included after our own
// header
#include <log-functions.h>
#include <hildon/hildon-program.h>

bool XAP_UnixHildonApp::s_bInitDone      = false;
bool XAP_UnixHildonApp::s_bRestoreNeeded = false;

static void s_topmost_changed_cb (HildonProgram *program,
								  GParamSpec *property_param,
								  gpointer data);

static void osso_hw_event_cb (osso_hw_state_t *state, gpointer data);
static gint osso_rpc_event_cb (const gchar     *interface,
                               const gchar     *method,
                               GArray          *arguments,
                               gpointer         data,
                               osso_rpc_t      *retval);

/*****************************************************************/
XAP_UnixHildonApp::XAP_UnixHildonApp(const char * szAppName)
	: XAP_UnixApp(szAppName),
	  m_pOsso(NULL),
	  m_pHildonProgram(NULL),
	  m_bHibernate(false)
{
}

XAP_UnixHildonApp::~XAP_UnixHildonApp()
{
	if (m_pOsso)
	{
		/* Unset callbacks */
		osso_hw_unset_event_cb (m_pOsso, NULL);
		osso_rpc_unset_default_cb_f (m_pOsso, osso_rpc_event_cb, NULL);

		/* Deinit osso */
		osso_deinitialize (m_pOsso);
		m_pOsso = NULL;
	}
}

bool XAP_UnixHildonApp::initialize(const char * szKeyBindingsKey,
								   const char * szKeyBindingsDefaultValue)
{
	osso_return_t ret;

	osso_log (LOG_INFO, "Initializing osso");

	//Initialize osso, for receive hardware signals

	// This prevents AbiWord being started from gdb. To start it from gdb in
	// Scratchbox you have todo
	// 
	// $ run-standalone.sh gdb /var/lib/install/usr/bin/AbiWord-2.6
	m_pOsso = osso_initialize ("com.abisource.abiword",
							   VERSION,
							   FALSE, 
							   NULL);

	if (m_pOsso == NULL)
	{
		osso_log (LOG_ERR, "Osso initialization failed" );
		UT_DEBUGMSG(("Osso initialization failed\n"));
		return false;
	}

	UT_DEBUGMSG(("m_pOsso 0x%p\n", m_pOsso));
	
	// Set handling changes in HW states. 
	ret = osso_hw_set_event_cb (m_pOsso, NULL, osso_hw_event_cb, this);
	if (ret != OSSO_OK)
	{
		osso_log (LOG_ERR, "Could not set callback for HW monitoring");
	}

	ret = osso_rpc_set_default_cb_f(m_pOsso, osso_rpc_event_cb, this);
	if (ret != OSSO_OK)
	{
		osso_log (LOG_ERR, "Could not set callback for receiving messages");
	}

	return XAP_UnixApp::initialize(szKeyBindingsKey,szKeyBindingsDefaultValue);
}

/*!
    Some of our dbus messages arrive before the app initialization is completed
    (in particular the message that tells us to restore from hibernation), and
    these need to be queued up for later. This function gets called from
    ap_UnixApp::main() just before gtk_main().
*/
void XAP_UnixHildonApp::processStartupQueue()
{
	UT_DEBUGMSG(("\n$$$$$$$$$$$ XAP_UnixHildonApp::precessStartupQueue() $$$$$$$$$$$$\n"));
	UT_return_if_fail( s_bInitDone );

	if(s_bRestoreNeeded)
	{
		UT_DEBUGMSG(("Restoring state\n"));
		retrieveState();
		s_bRestoreNeeded = false;
		setHibernate(false);
	}
}


/*!
    This methods returns an instance of HildonProgram object; there is only one
    instance of it for the applicaiton, and it is created by this function when
    it is first called.
*/
GObject *  XAP_UnixHildonApp::getHildonProgram() const
{
	if(!m_pHildonProgram)
	{
		m_pHildonProgram = G_OBJECT (hildon_program_get_instance ());
		UT_return_val_if_fail (m_pHildonProgram, NULL);

		g_object_set_data(m_pHildonProgram, "user_data",
						  const_cast<XAP_UnixHildonApp*>(this));

		g_signal_connect (m_pHildonProgram, "notify::is-topmost",
				G_CALLBACK (s_topmost_changed_cb),
				const_cast<XAP_UnixHildonApp*>(this));
	}

	return m_pHildonProgram;
}


bool XAP_UnixHildonApp::_saveState(XAP_StateData & sd)
{
	osso_state_t osd;
	osd.state_size = sizeof(XAP_StateData);
	osd.state_data = (gpointer)& sd;

	osso_return_t ret = osso_state_write(m_pOsso, &osd);

	UT_DEBUGMSG(("Save state called: ret %d\n", ret));
	return ( ret == OSSO_OK );
}

bool XAP_UnixHildonApp::_retrieveState(XAP_StateData & sd)
{
	/* we use the size of the stored data to differentiate between reall and stale state
	   data (since the state data survives after the app exits normally) */
	
	char c = 0;
	osso_state_t osd;
	osd.state_size = sizeof(char);
	osd.state_data = (gpointer)& c;

	osso_return_t ret = osso_state_read(m_pOsso, & osd);

	if(ret == OSSO_OK)
	{
		// this is not proper data ...
		return false;
	}

	// now try the real thing
	osd.state_size = sizeof(XAP_StateData);
	osd.state_data = (gpointer)& sd;
	
	ret = osso_state_read(m_pOsso, & osd);

	UT_DEBUGMSG(("Retrieve state called: ret %d\n", ret));
	return ( ret == OSSO_OK );
}

void XAP_UnixHildonApp::clearStateInfo()
{
	char c = 0;
	osso_state_t osd;
	osd.state_size = sizeof(char);
	osd.state_data = (gpointer)& c;

	osso_return_t ret = osso_state_write(m_pOsso, &osd);
	UT_ASSERT_HARMLESS( ret == OSSO_OK );
}

bool XAP_UnixHildonApp::isTopmost(void) const
{
	return hildon_program_get_is_topmost (HILDON_PROGRAM (m_pHildonProgram));
}

static void
_topmost_lose (XAP_UnixHildonApp *app,
		 HildonProgram *program)
{
	UT_DEBUGMSG(("%%%%%%%%%%%%%% topmost-status-lose %%%%%%%%%%%%%%%%%%\n"));

	// because it takes us something like 9s to start up, we will only
	// hibernated if the m_bHibernate flag is set. We set this flag in response
	// to low memory signal and clear it every time we get awaken from
	// hibernation
	if(app->getHibernate())
	{
		app->saveState(false);
		hildon_program_set_can_hibernate (HILDON_PROGRAM (program), TRUE);
	}
}

static void 
_topmost_acquire (XAP_UnixHildonApp *app,
		 HildonProgram *program)
{
	UT_DEBUGMSG(("%%%%%%%%%%%%%% topmost-status-acquire %%%%%%%%%%%%%%%%%%\n"));
	const char * pUntitled = "Untitled%d";
	const XAP_StringSet * pSS = app->getStringSet();
	if(pSS)
	{
		const char * p = pSS->getValue(XAP_STRING_ID_UntitledDocument);
		if(p && *p)
			pUntitled = p;
	}
	
	for(UT_sint32 i = 0; i < app->getFrameCount(); ++i)
	{
		XAP_Frame * pFrame = app->getFrame(i);
		if(!pFrame)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}

		AD_Document * pDoc = pFrame->getCurrentDoc();
		if(!pDoc)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}
		
		const char * pName = pFrame->getFilename();
		if(pName && *pName)
		{
			// for now, only do this for untitled docs use the localised name
			// for Untitled and compare len-2 chars (there is %d at the end of
			// the string)
			if(!strncmp(pName, pUntitled, strlen(pUntitled)-2))
			{
				const char * p = strstr(pName, "HIBERNATED.abw");

				// this was an untitled doc before we lost focus; make it look
				// like it still is
				if(p)
				{
					pDoc->clearFilename();
					pDoc->forceDirty();
					pFrame->updateTitle();
				}
			}
		}
	}
	
	// it would be better to do this in conditional fashion, after the user
	// modified the document, but this will do for now
	hildon_program_set_can_hibernate (HILDON_PROGRAM (program), FALSE);
}

static void
s_topmost_changed_cb (HildonProgram *program,
                      GParamSpec * /*property_param*/,
                      gpointer data)
{
	XAP_UnixHildonApp *pApp;
	UT_return_if_fail(data != NULL);
	pApp = static_cast<XAP_UnixHildonApp *>(data);

	if (hildon_program_get_is_topmost (program))
		_topmost_lose(pApp, program);
	else
		_topmost_acquire(pApp, program);
}


/* Depending on the state of hw, do something */
static void 
osso_hw_event_cb (osso_hw_state_t *state, 
		  gpointer    data)
{
	UT_DEBUGMSG(("osso_hw_event_cb() called\n"));
	XAP_UnixHildonApp *pApp;

	UT_return_if_fail (data != NULL);

	pApp = static_cast<XAP_UnixHildonApp *>(data);

	//signal shutdown received
	if (state->shutdown_ind) {
		for (UT_sint32 ndx = 0; ndx < pApp->getFrameCount(); ndx++) 
			pApp->getFrame(ndx)->close();
	}

	//signal save unsaved data received
	if (state->save_unsaved_data_ind) {
		if(XAP_UnixHildonApp::s_bInitDone)
		{
			UT_DEBUGMSG(("App ready, proceeding to save state ...\n"));
			pApp->saveState(false);
		}
		else
		{
			UT_DEBUGMSG(("App not ready, no state to save ...\n"));
		}
	}

	//signal memory low received
	if (state->memory_low_ind)
	{
		UT_DEBUGMSG(("Low memory hw signal\n"));
		if(XAP_UnixHildonApp::s_bInitDone)
		{
			UT_DEBUGMSG(("App ready, proceeding to save state ...\n"));
			pApp->setHibernate(true);

			// if we are not the active application, save state
			if(!pApp->isTopmost())
			{
				pApp->saveState(false);
			}
		}
		else
		{
			UT_DEBUGMSG(("App not ready, no state to save ...\n"));
		}
	}
	

	//signal system inactivity received
	if (state->system_inactivity_ind)
	{
		UT_DEBUGMSG(("System inactivity hw signal\n"));
	}
	
}

static gint osso_rpc_event_cb (const gchar     *interface,
                               const gchar     *method,
                               GArray          * /*arguments*/,
                               gpointer         data,
                               osso_rpc_t      * /*retval*/)
{
	UT_DEBUGMSG(("\n+++++++++ osso_rpc_event_cb() called; interface %s, method %s +++++++++++++++\n",
				 interface, method));
	
	XAP_UnixHildonApp *pApp;
	UT_return_val_if_fail (data != NULL, OSSO_ERROR);

	pApp = static_cast<XAP_UnixHildonApp *>(data);

	// this function can get called before the initialisation process is
	// finished, so some of these events need to be queued up
	if(!strcmp(method, "restored"))
	{
		if(XAP_UnixHildonApp::s_bInitDone)
		{
			UT_DEBUGMSG(("App ready, proceeding to restore state ...\n"));
			pApp->retrieveState();
			XAP_UnixHildonApp::s_bRestoreNeeded = false;
		}
		else
		{
			UT_DEBUGMSG(("App not ready, queuing up state restoration ...\n"));
			XAP_UnixHildonApp::s_bRestoreNeeded = true;
		}
	}
	else if(!strcmp(method, "top_application"))
	{
		if(XAP_UnixHildonApp::s_bInitDone)
		{
			UT_DEBUGMSG(("App ready, proceeding to restore state ...\n"));
			pApp->retrieveState();
			XAP_UnixHildonApp::s_bRestoreNeeded = false;
			pApp->setHibernate(false);
		}
		else
		{
			UT_DEBUGMSG(("App not ready, queuing up state restoration ...\n"));
			XAP_UnixHildonApp::s_bRestoreNeeded = true;
		}
	}
	
	
	return OSSO_OK;
}

