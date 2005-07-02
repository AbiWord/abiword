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


#ifndef __LIBABIWORD_PRIV_H__
#define __LIBABIWORD_PRIV_H__


#include <gtk/gtk.h>

#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "ap_UnixFrameImpl.h"

#ifdef ABIWORD_INTERNAL
#include "../abi-widget.h"
#else
#include <abiword/abi-widget.h>
#endif /* ABIWORD_INTERNAL */


class AP_WidgetFrameImpl : public AP_UnixFrameImpl {

public:
	AP_WidgetFrameImpl (AP_UnixFrame *pUnixFrame, 
						XAP_UnixApp  *pUnixApp);

	AbiWidget* getWidget 	  (void);
	void 	   setWidget 	  (AbiWidget *pAbiWidget);
	GtkWidget* getDrawingArea (void);

	XAP_FrameImpl* createInstance (XAP_Frame *pFrame, 
								   XAP_App 	 *pApp);

	UT_RGBColor getColorSelBackground (void) const;
	UT_RGBColor getColorSelForeground (void) const;
	
	// XAP_UnixFrameImpl

	void 		  createIMContext (GdkWindow *window);
	GtkIMContext* getIMContext 	  (void);

protected:

	void _showOrHideStatusbar (void);
	void _showOrHideToolbars  (void);

	void _refillToolbarsInFrameData (void);
	void _bindToolbars				(AV_View* pView);

	void _createWindow  (void);
	void _setWindowIcon (void);

	GtkWidget* _createDocumentWindow	(void);
	GtkWidget* _createStatusBarWindow	(void);

	void _setScrollRange (apufi_ScrollType scrollType, 
						  int 			   iValue, 
						  gfloat 		   fUpperLimit, 
						  gfloat 		   fSize); 

	void _setCursor (GR_Graphics::Cursor c);

	// XAP_FrameImpl

	bool _updateTitle (void);

private:
	AbiWidget *m_pAbiWidget;
};


class AP_WidgetFrame : public AP_UnixFrame {

public:
	AP_WidgetFrame (XAP_UnixApp 	   *pUnixApp, 
					AP_WidgetFrameImpl *pFrameImpl);

	AbiWidget* getWidget 	  (void);
	void 	   setWidget 	  (AbiWidget *pAbiWidget);
	AV_View*   getCurrentView (void);

	void setXScrollRange (void);
	void setYScrollRange (void);

	void setStatusMessage (const char *msg);

private:
	AbiWidget *m_pAbiWidget;
};


class AP_WidgetApp : public AP_UnixApp {

public:
	AP_WidgetApp (XAP_Args *pArgs, const char *szAppName);

	void setFrame (XAP_Frame *pFrame);
};


/*!
* \todo probably we'll need one frame(+impl) per widget
*/
typedef struct LibAbiWordPrivate {
	gboolean is_initialized;
	AP_WidgetApp *app;
	AP_WidgetFrame *frame;
	AP_WidgetFrameImpl *frame_impl;
};


#endif /* __LIBABIWORD_PRIV_H__ */
