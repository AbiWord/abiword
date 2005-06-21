/***************************************************************************
 *            xap_UnixHildonApp.cc
 *
 *  Thu May  5 11:12:21 2005
 *  Author   INDT - Renato Araujo <renato.filho@indt.org.br>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "xap_UnixHildonApp.h"


//TODO: implement dbus osso service
/*
 Receive D-BUS messages and handles them
*/
/*
static gint dbus_req_handler(const gchar *interface,
		      const gchar *method,
		      GArray *arguments, 
		      gpointer data, 
		      osso_rpc_t *retval)
{
	//TODO
	return 0;
}
*/

/* Depending on the state of hw, do something */
/*
static void hw_event_handler(osso_hw_state_t *state, gpointer data)
{
	if (state->reboot) 
	{
	// Rebooting 
	}
	if (state->memlow) 
	{
	// Memory low 
	}
	if (state->batlow)
	{
	// Battery low 
	}
	if (state->minact) 
	{
	// Minimum activity 
	}
}
*/

/*
 Osso topping callback
*/
static void osso_top_callback(const gchar *arguments, gpointer data)
{
	//TODO	
}

//*****************************************************************************************


HildonApp *XAP_UnixHildonApp::m_pHildonApp = NULL;

XAP_UnixHildonApp::XAP_UnixHildonApp()
//:m_osso_context(NULL)
{	
}

XAP_UnixHildonApp::~XAP_UnixHildonApp()
{
	terminate();	
}

/*
  Initiliza OSSO system
*/
bool XAP_UnixHildonApp::initialize()
{
/*		
	if (m_osso_context)
		return true;
	
	// Initialize Maemo application 
	m_osso_context = osso_initialize("osso_maemo_word", "0.0.1", TRUE, NULL);
	if (!m_osso_context)
		return false;


	gint ret = osso_application_set_top_cb(m_osso_context, osso_top_callback, NULL);
  
  	if (ret != OSSO_OK)
		return false;
	
	
	ret = osso_rpc_set_cb_f (m_osso_context,
			   	"com.nokia.osso_maemo_word_service",
				"/com/nokia/osso_maemo_word_service",
			   	"com.nokia.osso_maemo_word_service",
			   	dbus_req_handler,
			   	NULL);
	
	if (ret != OSSO_OK)
		return false;

	
	ret = osso_hw_set_event_cb(m_osso_context,
			     NULL,
			     hw_event_handler,
			     NULL);
	
	if (ret != OSSO_OK)
		return false;
	
*/

	m_pHildonApp = HILDON_APP(hildon_app_new());
	return true;
}

/*
  Terminate OSSO system
*/
void XAP_UnixHildonApp::terminate()
{
	/*
	if (m_osso_context)
	{	
		// Unset callbacks 
		osso_application_set_top_cb(m_osso_context, osso_top_callback, NULL);
		
		osso_rpc_unset_cb_f(m_osso_context, 
					"com.nokia.osso_maemo_word_service",
					"/com/nokia/osso_maemo_word_service",
					"com.nokia.osso_maemo_word_service",
					dbus_req_handler,
					NULL);
	
		osso_hw_unset_event_cb(m_osso_context,  NULL);
	  
		// Deinit osso 
		g_assert(m_osso_context);
		osso_deinitialize(m_osso_context);
		m_osso_context = NULL;
	}
	*/
}
