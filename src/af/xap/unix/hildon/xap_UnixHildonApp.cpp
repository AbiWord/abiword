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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include <stdio.h>
#include "xap_UnixHildonApp.h"
#include "xap_Frame.h"
#include <log-functions.h>

static void osso_hw_event_cb (osso_hw_state_t *state, gpointer data);
static void osso_top_event_cb (const gchar     *args, gpointer data);
static gint osso_rpc_event_cb (const gchar     *interface,
                               const gchar     *method,
                               GArray          *arguments,
                               gpointer         data,
                               osso_rpc_t      *retval);


/*****************************************************************/
XAP_UnixHildonApp::XAP_UnixHildonApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs, szAppName),
	  m_pOsso(NULL)
{
}

XAP_UnixHildonApp::~XAP_UnixHildonApp()
{
	if (m_pOsso) {
		/* Unset callbacks */
		osso_application_unset_top_cb (
			m_pOsso, 
			osso_top_event_cb, 
			NULL);


		osso_hw_unset_event_cb (
			m_pOsso,
			NULL);


		osso_rpc_unset_default_cb_f (
			m_pOsso, 
			osso_rpc_event_cb,
			NULL);

		/* Deinit osso */
		osso_deinitialize (m_pOsso);
		m_pOsso = NULL;
	}
}

bool XAP_UnixHildonApp::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	osso_return_t ret;

	osso_log (LOG_INFO, "Initializing osso");

	//Initialize osso, for receive hardware signals

	// This api is brain-dead -- if we are started via dbus, we have to pass this function
	// TRUE as activation parameter otherwise it will fail; if we are not started from
	// dbus, we have to pass it FALSE otherwise it will fail. Only one small problem: we
	// have no way of knowing why and how we were started.
	//
	// Normally we would be started by dbus, so hardcoding TRUE makes some sense, except this
	// prevents AbiWord being started from gdb. To start it from gdb in Scratchbox you have todo
	// 
	// $ run-standalone.sh gdb /var/lib/install/usr/bin/AbiWord-2.6

	m_pOsso = osso_initialize ("abiword",
							   VERSION,
							   TRUE, 
							   NULL);

	if (m_pOsso == NULL) {
		osso_log (LOG_ERR, " Osso initialization failed" );
		return false;
	}
	  
	// Set topping callback 
	ret = osso_application_set_top_cb (m_pOsso, osso_top_event_cb, this);
	if (ret != OSSO_OK)
		osso_log (LOG_ERR, "Could not set topping callback");

	// Set handling changes in HW states. 
	ret = osso_hw_set_event_cb (m_pOsso, NULL, osso_hw_event_cb, this);
	if (ret != OSSO_OK) {
		osso_log (LOG_ERR, "Could not set callback for HW monitoring");
	}

	ret = osso_rpc_set_default_cb_f(m_pOsso, osso_rpc_event_cb, this);
	if (ret != OSSO_OK) {
		osso_log (LOG_ERR, "Could not set callback for receiving messages");
	}

	return XAP_UnixApp::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);
}


void XAP_UnixHildonApp::_saveState(XAP_StateData & sd)
{
	UT_DEBUGMSG(("Save state called\n"));
	osso_state_t osd;
	osd.state_size = sizeof(XAP_StateData);
	osd.state_data = (gpointer)& sd;

	osso_return_t ret = osso_state_write(m_pOsso, &osd);

	UT_ASSERT_HARMLESS( ret == OSSO_OK );
}

void XAP_UnixHildonApp::_retrieveState(XAP_StateData & sd)
{
	UT_DEBUGMSG(("Retrieve state called\n"));
	osso_state_t osd;
	osd.state_size = sizeof(XAP_StateData);
	osd.state_data = (gpointer)& sd;
	
	osso_return_t ret = osso_state_read(m_pOsso, & osd);

	UT_ASSERT_HARMLESS( ret == OSSO_OK );
}


/* Depending on the state of hw, do something */
static void 
osso_hw_event_cb (osso_hw_state_t *state, 
		  gpointer    data)
{
	XAP_UnixHildonApp *pApp;

	g_return_if_fail (data != NULL);

	pApp = static_cast<XAP_UnixHildonApp *>(data);

	//signal shutdown received
	if (state->shutdown_ind) {
		for (unsigned int ndx=0; ndx < pApp->getFrameCount(); ndx++) 
			pApp->getFrame(ndx)->close();
	}

	//signal save unsaved data received
	if (state->save_unsaved_data_ind) {
	    // TODO: can we just save the file? the user might not like that;
	    // IMO we need to save to some kind of backup or tmp file.
	}

	//signal memory low received
	//if (state->memory_low_ind);

	//signal system inactivity received
	//if (state->system_inactivity_ind);
}

static void osso_top_event_cb (const gchar     *args, gpointer data)
{
	return;
}

static gint osso_rpc_event_cb (const gchar     *interface,
                               const gchar     *method,
                               GArray          *arguments,
                               gpointer         data,
                               osso_rpc_t      *retval)
{
	XAP_UnixHildonApp *pApp;
	g_return_val_if_fail (data != NULL, OSSO_ERROR);

	pApp = static_cast<XAP_UnixHildonApp *>(data);

	if(strcmp(method, "restored"))
	{
		pApp->retrieveState();
	}
	
	return OSSO_OK;
}



