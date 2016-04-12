/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003-2004, 2009 Hubert Figuiere
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


#ifndef XAP_COCOAAPP_H
#define XAP_COCOAAPP_H

//  BAD HACK BAD HACK I can't include Foundation/NSTimer.h
//  because AP_Args somehow won't compile Obj-C.
typedef double NSTimeInterval;

#include <unistd.h>
#include <sys/stat.h>
#include "xap_App.h"
#include "xap_CocoaDialogFactory.h"
#include "xap_Cocoa_TB_CFactory.h"

class XAP_Args;
class XAP_CocoaToolbar_Icons;
class AV_View;
class EV_CocoaMenuBar;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_CocoaApp : public XAP_App
{
public:
	XAP_CocoaApp(const char* szAppName);
	virtual ~XAP_CocoaApp();

	virtual const char * 					getDefaultEncoding () const;

	virtual bool							initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);
	virtual XAP_Frame * 					newFrame() = 0;
	virtual void							reallyExit();
	virtual void							notifyFrameCountChange ();

	virtual XAP_DialogFactory *				getDialogFactory();
	virtual XAP_Toolbar_ControlFactory *	getControlFactory();
	virtual const XAP_StringSet *			getStringSet() const = 0;
	virtual const char *					getAbiSuiteAppDir() const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool							canPasteFromClipboard() = 0;
	virtual const char *					getUserPrivateDirectory() const;
	virtual bool							findAbiSuiteBundleFile(std::string & path, const char * filename, const char * subdir = 0); // checks only bundle
	virtual bool							findAbiSuiteLibFile(std::string & path, const char * filename, const char * subdir = 0);
	virtual bool							findAbiSuiteAppFile(std::string & path, const char * filename, const char * subdir = 0); // doesn't check user-dir

	virtual void							setSelectionStatus(AV_View * pView) = 0;
	virtual void							clearSelection() = 0;
	virtual bool							getCurrentSelection(const char** formatList,
																void ** ppData, UT_uint32 * pLen,
																const char **pszFormatFound) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;

	typedef enum
	{
		GEOMETRY_FLAG_POS = 	1 << 0,
		GEOMETRY_FLAG_SIZE = 	1 << 1
	} windowGeometryFlags;

	struct windowGeometry
	{
		int x, y;
		UT_uint32 width, height;
		XAP_CocoaApp::windowGeometryFlags flags;
	};

	virtual	void					setGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
												windowGeometryFlags flags);
	virtual	void					getGeometry(int * x, int * y, UT_uint32 * width, UT_uint32 * height,
												windowGeometryFlags * flags);

	void							setTimeOfLastEvent(NSTimeInterval eventTime);
	NSTimeInterval					getTimeOfLastEvent() const { return m_eventTime; };
    virtual BidiSupportType         theOSHasBidiSupport() const;
	EV_CocoaMenuBar*				getCocoaMenuBar(void) const { return m_pCocoaMenu; };
	XAP_Frame * 					_getFrontFrame(void);
	XAP_CocoaToolbar_Icons *		getToolbarIcons () const { return m_pCocoaToolbarIcons; }
protected:
	virtual const char*          _findNearestFont(const char* pszFontFamily,
												const char* pszFontStyle,
												const char* pszFontVariant,
												const char* pszFontWeight,
												const char* pszFontStretch,
												const char* pszFontSize,
												const char* pszLang);
	bool							_loadFonts();
	void							_setAbiSuiteLibDir();
private:
	XAP_CocoaToolbar_Icons *		m_pCocoaToolbarIcons;
	AP_CocoaDialogFactory			m_dialogFactory;
	AP_CocoaToolbar_ControlFactory	m_controlFactory;

	windowGeometry			m_geometry;
	NSTimeInterval			m_eventTime; // e->time field of a recent X event
										 // (we use this to sync clipboard
										 // operations with the server).

protected:				// TODO move that to private
	EV_CocoaMenuBar*			m_pCocoaMenu;
	const char*			m_szMenuLayoutName;
	const char*			m_szMenuLabelSetName;
};

#endif /* XAP_COCOAAPP_H */
