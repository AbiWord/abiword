/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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


#ifndef XAP_UNIXAPP_H
#define XAP_UNIXAPP_H

#include "xap_App.h"
#include "xap_UnixDialogFactory.h"
#include "xap_Unix_TB_CFactory.h"
#include "xap_UnixFontManager.h"
#include "xap_Strings.h"

#include <gdk/gdk.h>

class XAP_Args;
class AP_UnixToolbar_Icons;
class AV_View;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_UnixApp : public XAP_App
{
public:
	XAP_UnixApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_UnixApp(void);

	virtual bool							initialize(void);
	virtual XAP_Frame * 					newFrame(void) = 0;
	virtual void							reallyExit(void);

	virtual XAP_DialogFactory *				getDialogFactory(void);
	virtual XAP_Toolbar_ControlFactory *	getControlFactory(void);
	virtual const XAP_StringSet *			getStringSet(void) const = 0;
	virtual const char *					getAbiSuiteAppDir(void) const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool							canPasteFromClipboard(void) = 0;
	virtual const char *					getUserPrivateDirectory(void);

	virtual void							setSelectionStatus(AV_View * pView) = 0;
	virtual void							clearSelection(void) = 0;
	virtual bool							getCurrentSelection(const char** formatList,
																void ** ppData, UT_uint32 * pLen,
																const char **pszFormatFound) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;

	XAP_UnixFontManager *					getFontManager(void);

	typedef enum
	{
		GEOMETRY_FLAG_POS = 	1 << 0,
		GEOMETRY_FLAG_SIZE = 	1 << 1
	} windowGeometryFlags;
	
	struct windowGeometry
	{
		gint x, y;
		guint width, height;
		XAP_UnixApp::windowGeometryFlags flags;
	};
	
	virtual	void					setGeometry(gint x, gint y, guint width, guint height,
												windowGeometryFlags flags);
	virtual	void					getGeometry(gint * x, gint * y, guint * width, guint * height,
												windowGeometryFlags * flags);

	void							setTimeOfLastEvent(guint32 eventTime);
	guint32							getTimeOfLastEvent(void) const { return m_eventTime; };
	
protected:
	bool							_loadFonts(void);
	void							_setAbiSuiteLibDir(void);

	AP_UnixToolbar_Icons *			m_pUnixToolbarIcons;
	AP_UnixDialogFactory			m_dialogFactory;
	AP_UnixToolbar_ControlFactory	m_controlFactory;
	XAP_UnixFontManager *			m_fontManager;

	windowGeometry			m_geometry;
	guint32					m_eventTime; // e->time field of a recent X event
										 // (we use this to sync clipboard
										 // operations with the server).
};

#endif /* XAP_UNIXAPP_H */
