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

#include "ev_EditMethod.h"
#include "xap_UnixHildonApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

// something wrong with these headers -- they need to be inculded after our own header
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
	  m_pHildonAppWidget(NULL),
	  m_imContext(NULL)
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

	g_object_unref (G_OBJECT (m_imContext));

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


/*!
    Because of the peculiar way hildon handles windows and views, we need a custom
    function for the delete_event
*/
static gint s_delete_event(GtkWidget * w, GdkEvent * /*event*/, gpointer /*data*/)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(g_object_get_data(G_OBJECT(w), "user_data"));
	UT_return_val_if_fail( pThis, FALSE );

	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_val_if_fail( pHView, FALSE );

	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(pHView), "user_data"));
	XAP_Frame* pFrame = pFrameImpl->getFrame();
	XAP_UnixHildonApp * pApp = static_cast<XAP_UnixHildonApp*>(XAP_App::getApp());

	if(pApp->isBonoboRunning())
		return FALSE;

	UT_uint32 iFrameCount = pApp->getFrameCount();
	
	const EV_Menu_ActionSet * pMenuActionSet = pApp->getMenuActionSet();
	UT_ASSERT(pMenuActionSet);

	const EV_EditMethodContainer * pEMC = pApp->getEditMethodContainer();
	UT_ASSERT(pEMC);

	// was "closeWindow", TRUE, FALSE
	const EV_EditMethod * pEM = pEMC->findEditMethodByName("closeWindowX");
	UT_ASSERT(pEM);

	if (pEM)
	{
		if (pEM->Fn(pFrame->getCurrentView(),NULL))
		{
			// returning FALSE means destroy the window, continue along the
			// chain of Gtk destroy events
			
			// with hildon, our frames are not proper windows, but behave more like tabs
			// in a single window. Pressing the 'close' button, however, closes the main
			// window, not the individual tab. So, if there are frames left, we must
			// return true to stop the processing chain. We also have to update the window
			// title 
			
			if(iFrameCount > 1)
			{
				// switch the main window to a different view
				GtkWidget * pHildonAppWidget = pApp->getHildonAppWidget();
				UT_return_val_if_fail( pHildonAppWidget, TRUE );

				pFrame = pApp->getFrame(0);
				UT_return_val_if_fail( pFrame, TRUE );

				pFrameImpl = static_cast<XAP_UnixFrameImpl*>(pFrame->getFrameImpl());
				UT_return_val_if_fail( pFrameImpl, TRUE );
				
				hildon_app_set_appview(HILDON_APP(pHildonAppWidget), HILDON_APPVIEW(pFrameImpl->getTopLevelWindow()));
				
				return TRUE;
			}
			else
				return FALSE;
		}
	}

	// returning TRUE means do NOT destroy the window; halt the message
	// chain so it doesn't see destroy
	return TRUE;
}

/*!
   HildonAppViews, which represent our frames, do not receive focus_in and _out events;
   intstead those events are caught by HildonApp; consequently our event handler needs to
   call the frame handler implemented in XAP_UnixFrameImpl
*/
static gboolean s_focus_in_event(GtkWidget *w,GdkEvent *event, gpointer user_data)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(g_object_get_data(G_OBJECT(w), "user_data"));
	UT_return_val_if_fail( pThis, FALSE );

	UT_DEBUGMSG(("\n===========================\nHildonApp focus_in_event: pThis 0x%x\n=========================\n",
				 pThis));

	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_val_if_fail( pHView, FALSE );
	
	return XAP_UnixFrameImpl::_fe::focus_in_event(GTK_WIDGET(pHView), event, user_data);
}

static gboolean s_focus_out_event(GtkWidget *w,GdkEvent *event, gpointer user_data)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(g_object_get_data(G_OBJECT(w), "user_data"));
	UT_return_val_if_fail( pThis, FALSE );

	UT_DEBUGMSG(("\n===========================\nHildonApp focus_out_event: pThis 0x%x\n=========================\n",
				 pThis));

	
	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_val_if_fail( pHView, FALSE );

	return XAP_UnixFrameImpl::_fe::focus_out_event(GTK_WIDGET(pHView), event, user_data);
}

/*!
    All input from the virtual keyboard operates on the HildonApp widget; the individual
    frames do not get any signals. Consequently, we create one shared input context for
    all our frames in the XAP_UnixHildonApp and register callbacks for it from here; we
    then forward all events to the normal callbacks implemented in XAP_UnixFrameImp.
*/
static void s_imPreeditChanged_cb (GtkIMContext *context, gpointer data)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(data);
	UT_return_if_fail( pThis );

	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_if_fail( pHView );

	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(pHView), "user_data"));
	UT_return_if_fail( pFrameImpl );

	XAP_UnixFrameImpl::_imPreeditChanged_cb (context, (gpointer)pFrameImpl);
}

static gint s_imRetrieveSurrounding_cb (GtkIMContext *context, gpointer data)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(data);
	UT_return_val_if_fail( pThis, FALSE );

	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_val_if_fail( pHView, FALSE );

	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(pHView), "user_data"));
	UT_return_val_if_fail( pFrameImpl, FALSE );

	return XAP_UnixFrameImpl::_imRetrieveSurrounding_cb (context, (gpointer)pFrameImpl);
}

static gint s_imDeleteSurrounding_cb (GtkIMContext *slave, gint offset, gint n_chars, gpointer data)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(data);
	UT_return_val_if_fail( pThis, FALSE );

	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_val_if_fail( pHView, FALSE );

	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(pHView), "user_data"));
	UT_return_val_if_fail( pFrameImpl, FALSE );

	return XAP_UnixFrameImpl::_imDeleteSurrounding_cb (slave, offset, n_chars , (gpointer)pFrameImpl);
}

static void s_imCommit_cb(GtkIMContext *imc, const gchar *text, gpointer data)
{
	XAP_UnixHildonApp * pThis = static_cast<XAP_UnixHildonApp*>(data);
	UT_return_if_fail( pThis );

	HildonAppView * pHView = hildon_app_get_appview(HILDON_APP(pThis->getHildonAppWidget()));
	UT_return_if_fail( pHView );

	XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(g_object_get_data(G_OBJECT(pHView), "user_data"));
	UT_return_if_fail( pFrameImpl );

	XAP_UnixFrameImpl::_imCommit_cb (imc, text, (gpointer)pFrameImpl);
}

/*!
    This methods returns an instance of HildonApp widget; there is only one instance of it
    for the applicaiton, and it is created by this function when it is first called.
*/
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

		g_object_set_data(G_OBJECT(m_pHildonAppWidget), "user_data", const_cast<XAP_UnixHildonApp*>(this)); 
	
		gtk_window_set_role(GTK_WINDOW(m_pHildonAppWidget), "topLevelWindow");		
		g_object_set_data(G_OBJECT(m_pHildonAppWidget), "toplevelWindow", m_pHildonAppWidget);

		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "realize",
						 G_CALLBACK(XAP_UnixFrameImpl::_fe::realize), NULL);

		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "unrealize",
						 G_CALLBACK(XAP_UnixFrameImpl::_fe::unrealize), NULL);

		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "size_allocate",
						 G_CALLBACK(XAP_UnixFrameImpl::_fe::sizeAllocate), NULL);
		
		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "delete_event",
						 G_CALLBACK(s_delete_event), NULL);
		
		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "destroy",
						 G_CALLBACK(XAP_UnixFrameImpl::_fe::destroy), NULL);


		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "focus_in_event",
						 G_CALLBACK(s_focus_in_event), NULL);
		g_signal_connect(G_OBJECT(m_pHildonAppWidget), "focus_out_event",
						 G_CALLBACK(s_focus_out_event), NULL);


		gtk_widget_realize(m_pHildonAppWidget);

		m_imContext = gtk_im_multicontext_new();
		gtk_im_context_set_use_preedit (m_imContext, FALSE); 
		gtk_im_context_set_client_window(m_imContext, m_pHildonAppWidget->window);
		
		g_signal_connect(G_OBJECT(m_imContext), "commit", 
						 G_CALLBACK(s_imCommit_cb), const_cast<XAP_UnixHildonApp*>(this));
		
		g_signal_connect (m_imContext, "preedit_changed",
					  G_CALLBACK (s_imPreeditChanged_cb), const_cast<XAP_UnixHildonApp*>(this));
		
		g_signal_connect (m_imContext, "retrieve_surrounding",
						  G_CALLBACK (s_imRetrieveSurrounding_cb), const_cast<XAP_UnixHildonApp*>(this));
		
		g_signal_connect (m_imContext, "delete_surrounding",
						  G_CALLBACK (s_imDeleteSurrounding_cb), const_cast<XAP_UnixHildonApp*>(this));

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


