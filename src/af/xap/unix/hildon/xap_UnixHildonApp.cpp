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
#include <hildon-widgets/hildon-app.h>

static void osso_hw_event_cb (osso_hw_state_t *state, gpointer data);
static gint osso_rpc_event_cb (const gchar     *interface,
                               const gchar     *method,
                               GArray          *arguments,
                               gpointer         data,
                               osso_rpc_t      *retval);

static void osso_exit_event_cb (gboolean die_now, gpointer data);

/*****************************************************************/
XAP_UnixHildonApp::XAP_UnixHildonApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs, szAppName),
	  m_pOsso(NULL),
	  m_pHildonAppWidget(NULL)
{
}

XAP_UnixHildonApp::~XAP_UnixHildonApp()
{
	if (m_pOsso) {
		/* Unset callbacks */
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

	// This prevents AbiWord being started from gdb. To start it from gdb in Scratchbox
	// you have todo
	// 
	// $ run-standalone.sh gdb /var/lib/install/usr/bin/AbiWord-2.6
	m_pOsso = osso_initialize ("abiword",
							   VERSION,
							   FALSE, 
							   NULL);

	if (m_pOsso == NULL) {
		osso_log (LOG_ERR, "Osso initialization failed" );
		UT_DEBUGMSG(("Osso initialization failed\n"));
		return false;
	}

	UT_DEBUGMSG(("m_pOsso 0x%x\n", m_pOsso));
	
	// Set handling changes in HW states. 
	ret = osso_hw_set_event_cb (m_pOsso, NULL, osso_hw_event_cb, this);
	if (ret != OSSO_OK) {
		osso_log (LOG_ERR, "Could not set callback for HW monitoring");
	}

	ret = osso_rpc_set_default_cb_f(m_pOsso, osso_rpc_event_cb, this);
	if (ret != OSSO_OK) {
		osso_log (LOG_ERR, "Could not set callback for receiving messages");
	}

	
	ret = osso_application_set_exit_cb(m_pOsso, osso_exit_event_cb, this);
	if (ret != OSSO_OK) {
		osso_log (LOG_ERR, "Could not set exit callback\n");
	}

		
	return XAP_UnixApp::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);
}


GtkWidget *  XAP_UnixHildonApp::getHildonAppWidget() const
{
	if(!m_pHildonAppWidget)
	{
		m_pHildonAppWidget = hildon_app_new();
		UT_return_val_if_fail( m_pHildonAppWidget, NULL );

		hildon_app_set_killable(HILDON_APP(m_pHildonAppWidget), FALSE);
		hildon_app_set_autoregistration(HILDON_APP(m_pHildonAppWidget), TRUE);
		hildon_app_set_title(HILDON_APP(m_pHildonAppWidget), getApplicationTitleForTitleBar());
		hildon_app_set_two_part_title(HILDON_APP(m_pHildonAppWidget), TRUE);

		gtk_window_set_role(GTK_WINDOW(m_pHildonAppWidget), "topLevelWindow");		
		g_object_set_data(G_OBJECT(m_pHildonAppWidget), "toplevelWindow", m_pHildonAppWidget);
	}

	return m_pHildonAppWidget;
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
	UT_DEBUGMSG(("osso_hw_event_cb() called\n"));
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
	if (state->memory_low_ind)
	{
		UT_DEBUGMSG(("Low memory hw signal\n"));
	}
	

	//signal system inactivity received
	if (state->system_inactivity_ind)
	{
		UT_DEBUGMSG(("System inactivity hw signal\n"));
	}
	
}

static gint osso_rpc_event_cb (const gchar     *interface,
                               const gchar     *method,
                               GArray          *arguments,
                               gpointer         data,
                               osso_rpc_t      *retval)
{
	UT_DEBUGMSG(("osso_rpc_event_cb() called; interface %s, method %s\n",
				 interface, method));
	
	XAP_UnixHildonApp *pApp;
	g_return_val_if_fail (data != NULL, OSSO_ERROR);

	pApp = static_cast<XAP_UnixHildonApp *>(data);

	if(!strcmp(method, "restored"))
	{
		pApp->retrieveState();
	}
	
	return OSSO_OK;
}


static void osso_exit_event_cb (gboolean /*die_now*/, gpointer /*data*/)
{
	UT_DEBUGMSG(("osso_exit_event_cb() called\n"));
}


