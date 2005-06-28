/* LibAbi
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


#ifndef __AP_WIDGET_APP_H__
#define __AP_WIDGET_APP_H__


#define UT_DEBUGMSG(M) printf M


#include <gtk/gtk.h>

#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "ap_UnixFrameImpl.h"

#include "abi-widget-priv.h"


class AP_WidgetFrameImpl : public AP_UnixFrameImpl {
public:
	AP_WidgetFrameImpl (AbiWidget *pAbiWidget, 
						AP_UnixFrame *pUnixFrame, 
						XAP_UnixApp *pUnixApp)
	  : AP_UnixFrameImpl (pUnixFrame, pUnixApp), 
		m_pAbiWidget (pAbiWidget)
	{
		UT_DEBUGMSG (("AP_WidgetFrameImpl() frame-impl: %x, dreawing-area: %x, window: %x\n", this, pAbiWidget->widgets->d_area, pAbiWidget->widgets->d_area->window));
		m_dArea = pAbiWidget->widgets->d_area;
	}

	virtual GtkWidget* getDrawingArea (void) 
	{ 
		UT_DEBUGMSG (("AP_WidgetFrameImpl::getDrawingArea() %x\n", m_pAbiWidget->widgets->d_area));
		return m_pAbiWidget->widgets->d_area; 
	}

	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp) { UT_DEBUGMSG (("AP_WidgetFrameImpl::createInstance()\n")); }

	virtual UT_RGBColor getColorSelBackground () const { UT_DEBUGMSG (("AP_WidgetFrameImpl::getColorSelBackground()\n")); }
	virtual UT_RGBColor getColorSelForeground () const { UT_DEBUGMSG (("AP_WidgetFrameImpl::getColorSelForeground()\n")); }
	
	// XAP_UnixFrameImpl

	GtkIMContext * getIMContext() { return m_imContext; }

 protected:

	virtual void _showOrHideStatusbar(void) { UT_DEBUGMSG (("AP_WidgetFrameImpl::_showOrHideStatusbar()\n")); }
	virtual void _showOrHideToolbars(void) { UT_DEBUGMSG (("AP_WidgetFrameImpl::_showOrHideToolbars()\n")); }

	virtual void _refillToolbarsInFrameData() { UT_DEBUGMSG (("AP_WidgetFrameImpl::_refillToolbarsInFrameData()\n")); }
	virtual void _bindToolbars(AV_View * pView) { UT_DEBUGMSG (("AP_WidgetFrameImpl::_bindToolbars()\n")); }
	virtual void _createWindow() { UT_DEBUGMSG (("AP_WidgetFrameImpl::_createWindow()\n")); }

	virtual GtkWidget * _createDocumentWindow() { UT_DEBUGMSG (("AP_WidgetFrameImpl::_createDocumentWindow()\n")); }
	virtual GtkWidget * _createStatusBarWindow() { UT_DEBUGMSG (("AP_WidgetFrameImpl::_createStatusBarWindow()\n")); }

	virtual void _setWindowIcon() { UT_DEBUGMSG (("AP_WidgetFrameImpl::_setWindowIcon()\n")); }
	virtual void _setScrollRange(apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize) { UT_DEBUGMSG (("AP_WidgetFrameImpl::_setScrollRange()\n")); } 

	virtual void _setCursor (GR_Graphics::Cursor c) { UT_DEBUGMSG (("AP_WidgetFrameImpl::_setCursor()\n")); }

	// XAP_FrameImpl

	virtual bool _updateTitle() { UT_DEBUGMSG (("AP_WidgetFrameImpl::_updateTitle()\n")); return TRUE; }

private:
	AbiWidget *m_pAbiWidget;
};


class AP_WidgetFrame : public AP_UnixFrame {
public:
	AP_WidgetFrame (AbiWidget *pAbiWidget,
					XAP_UnixApp *pUnixApp, 
					AP_WidgetFrameImpl *pFrameImpl)
	  : AP_UnixFrame (pUnixApp, pFrameImpl),
		m_pAbiWidget (pAbiWidget)
	{}

	AV_View* getCurrentView () 
	{
		UT_DEBUGMSG (("AP_WidgetFrameImpl::getCurrentView() %x\n", m_pAbiWidget->view));
		return m_pAbiWidget->view;
	}

	virtual void setXScrollRange (void)
	{
		abi_widget_priv_set_x_scroll_range (m_pAbiWidget);
	}

	virtual void setYScrollRange (void)
	{
		abi_widget_priv_set_y_scroll_range (m_pAbiWidget);
	}

	virtual void setStatusMessage(const char * msg)
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

private:
	AbiWidget *m_pAbiWidget;
};


class AP_WidgetApp : public AP_UnixApp {
public:
	AP_WidgetApp (XAP_Args *pArgs, const char *szAppName)
	  : AP_UnixApp (pArgs, szAppName)
	{}

	void setFrame (XAP_Frame *pFrame) { m_vecFrames.addItem (pFrame); }
};


#endif /*  __AP_WIDGET_APP_H__ */
