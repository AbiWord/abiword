/*
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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


#include <gtk/gtk.h>
#include "libabiword.h"
#ifdef ABIWORD_INTERNAL
#include "private/libabiword-priv.h"
#else
#include <abiword/private/libabiword-priv.h>
#endif /* ABIWORD_INTERNAL */


#define UT_DEBUGMSG(M) printf M


LibAbiWordPrivate libabiword_priv;


void 
libabiword_init (void)
{
	const char *argv[1];
	XAP_Args *args;

	if (libabiword_priv.is_initialized) {
		return;
	}

	argv[0] = "AbiWidget";
	args = new XAP_Args (1, argv);

	libabiword_priv.app = new AP_WidgetApp (args, argv[0]);
	libabiword_priv.app->initialize (TRUE);

	libabiword_priv.frame_impl = new AP_WidgetFrameImpl (NULL, libabiword_priv.app);

	libabiword_priv.frame = new AP_WidgetFrame (libabiword_priv.app, libabiword_priv.frame_impl);
	libabiword_priv.frame->setFrameImpl (libabiword_priv.frame_impl);
	libabiword_priv.frame->initialize ();

	libabiword_priv.app->setFrame (libabiword_priv.frame);

	UT_DEBUGMSG (("init() app: %x, frame: %x, frame_impl: %x\n", 
				  libabiword_priv.app, libabiword_priv.frame, libabiword_priv.frame_impl));

	libabiword_priv.is_initialized = TRUE;
}

void 
libabiword_shutdown (void)
{
	/* TODO clean up */

	libabiword_priv.is_initialized = FALSE;
}



AP_WidgetFrameImpl::AP_WidgetFrameImpl (AP_UnixFrame *pUnixFrame, 
										XAP_UnixApp  *pUnixApp)
  : AP_UnixFrameImpl (pUnixFrame, pUnixApp),
	m_pAbiWidget (NULL)
{}

AbiWidget*
AP_WidgetFrameImpl::getWidget (void)
{
	return m_pAbiWidget;
}

void
AP_WidgetFrameImpl::setWidget (AbiWidget *pAbiWidget)
{
	m_pAbiWidget = pAbiWidget;
	UT_DEBUGMSG (("AP_WidgetFrameImpl() frame-impl: %x, dreawing-area: %x, window: %x\n", 
				  this, pAbiWidget->widgets->d_area, pAbiWidget->widgets->d_area->window));
	m_dArea = pAbiWidget->widgets->d_area;
}

GtkWidget* 
AP_WidgetFrameImpl::getDrawingArea (void)
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::getDrawingArea() %x\n", m_pAbiWidget->widgets->d_area));
	return m_pAbiWidget->widgets->d_area; 
}

XAP_FrameImpl * 
AP_WidgetFrameImpl::createInstance(XAP_Frame *pFrame, XAP_App *pApp) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::createInstance()\n")); 
}

UT_RGBColor 
AP_WidgetFrameImpl::getColorSelBackground (void) const 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::getColorSelBackground()\n")); 
}

UT_RGBColor 
AP_WidgetFrameImpl::getColorSelForeground (void) const 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::getColorSelForeground()\n")); 
}

void
AP_WidgetFrameImpl::createIMContext (GdkWindow *window)
{
	_createIMContext (window);
}

GtkIMContext* 
AP_WidgetFrameImpl::getIMContext (void) 
{ 
	return m_imContext; 
}

void 
AP_WidgetFrameImpl::_showOrHideStatusbar (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_showOrHideStatusbar()\n")); 
}

void 
AP_WidgetFrameImpl::_showOrHideToolbars (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_showOrHideToolbars()\n")); 
}

void 
AP_WidgetFrameImpl::_refillToolbarsInFrameData (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_refillToolbarsInFrameData()\n")); 
}

void 
AP_WidgetFrameImpl::_bindToolbars (AV_View * pView) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_bindToolbars()\n")); 
}

void 
AP_WidgetFrameImpl::_createWindow (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_createWindow()\n")); 
}

GtkWidget* 
AP_WidgetFrameImpl::_createDocumentWindow (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_createDocumentWindow()\n")); 
}

GtkWidget* 
AP_WidgetFrameImpl::_createStatusBarWindow (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_createStatusBarWindow()\n")); 
}

void 
AP_WidgetFrameImpl::_setWindowIcon (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_setWindowIcon()\n")); 
}

void 
AP_WidgetFrameImpl::_setScrollRange (apufi_ScrollType scrollType, 
									 int 			  iValue, 
									 gfloat 		  fUpperLimit, 
									 gfloat 		  fSize)
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_setScrollRange()\n")); 
} 

void 
AP_WidgetFrameImpl::_setCursor (GR_Graphics::Cursor c) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_setCursor()\n")); 
}

bool 
AP_WidgetFrameImpl::_updateTitle (void) 
{ 
	UT_DEBUGMSG (("AP_WidgetFrameImpl::_updateTitle()\n")); return TRUE; 
}



AP_WidgetFrame::AP_WidgetFrame (XAP_UnixApp 	   *pUnixApp, 
								AP_WidgetFrameImpl *pFrameImpl)
  : AP_UnixFrame (pUnixApp, pFrameImpl),
	m_pAbiWidget (NULL)
{}

AbiWidget*
AP_WidgetFrame::getWidget (void)
{
	return m_pAbiWidget;
}

void
AP_WidgetFrame::setWidget (AbiWidget *pAbiWidget)
{
	m_pAbiWidget = pAbiWidget;
}

AV_View* 
AP_WidgetFrame::getCurrentView (void) 
{
	UT_DEBUGMSG (("AP_WidgetFrameImpl::getCurrentView() %x\n", m_pAbiWidget->view));
	return m_pAbiWidget->view;
}

void 
AP_WidgetFrame::setXScrollRange (void)
{
	abi_widget_priv_set_x_scroll_range (m_pAbiWidget);
}

void 
AP_WidgetFrame::setYScrollRange (void)
{
	abi_widget_priv_set_y_scroll_range (m_pAbiWidget);
}

void 
AP_WidgetFrame::setStatusMessage(const char * msg)
{	
	static char *last_msg = NULL;

	if (msg && !last_msg ||
		!msg && last_msg ||
		msg && last_msg && strcmp (msg, last_msg) != 0) {

		if (last_msg) {
			g_free (last_msg);
			last_msg = NULL;
		}

		if (msg)
			last_msg = g_strdup (msg);

		// TODO fire event
		printf ("AP_WidgetFrame::setStatusMessage() '%s'\n", msg);
	}
}



AP_WidgetApp::AP_WidgetApp (XAP_Args   *pArgs, 
							const char *szAppName)
  : AP_UnixApp (pArgs, szAppName)
{}

void 
AP_WidgetApp::setFrame (XAP_Frame *pFrame) 
{ 
	m_vecFrames.addItem (pFrame); 
}
