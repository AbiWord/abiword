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

class XAP_Args;
class AP_UnixToolbar_Icons;
class AV_View;
class XAP_UnixFontManager;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_UnixApp : public XAP_App
{
public:
	XAP_UnixApp(XAP_Args* pArgs, const char* szAppName);
	virtual ~XAP_UnixApp();

	virtual bool							initialize();
	virtual XAP_Frame * 					newFrame() = 0;
	virtual void							reallyExit();

	virtual XAP_DialogFactory *				getDialogFactory();
	virtual XAP_Toolbar_ControlFactory *	getControlFactory();
	virtual const XAP_StringSet *			getStringSet() const = 0;
	virtual const char *					getAbiSuiteAppDir() const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool							canPasteFromClipboard() = 0;
	virtual const char *					getUserPrivateDirectory();

	virtual void							setSelectionStatus(AV_View * pView) = 0;
	virtual void							clearSelection() = 0;
	virtual bool							getCurrentSelection(const char** formatList,
																void ** ppData, UT_uint32 * pLen,
																const char **pszFormatFound) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;

	XAP_UnixFontManager *					getFontManager();

	typedef enum
	{
		GEOMETRY_FLAG_POS = 	1 << 0,
		GEOMETRY_FLAG_SIZE = 	1 << 1
	} windowGeometryFlags;
	
	struct windowGeometry
	{
		int x, y;
		UT_uint32 width, height;
		XAP_UnixApp::windowGeometryFlags flags;
	};
	
	virtual	void					setGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
												windowGeometryFlags flags);
	virtual	void					getGeometry(int * x, int * y, UT_uint32 * width, UT_uint32 * height,
												windowGeometryFlags * flags);

	void							setTimeOfLastEvent(UT_uint32 eventTime);
	UT_uint32	   					getTimeOfLastEvent() const { return m_eventTime; };
	
protected:
	bool							_loadFonts();
	void							_setAbiSuiteLibDir();

	AP_UnixToolbar_Icons *			m_pUnixToolbarIcons;
	AP_UnixDialogFactory			m_dialogFactory;
	AP_UnixToolbar_ControlFactory	m_controlFactory;
	XAP_UnixFontManager *			m_fontManager;

	windowGeometry			m_geometry;
	UT_uint32					m_eventTime; // e->time field of a recent X event
										 // (we use this to sync clipboard
										 // operations with the server).
};

#endif /* XAP_UNIXAPP_H */
