/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/*
 * Port to Maemo Development Platform
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */


#ifndef XAP_UNIXAPP_H
#define XAP_UNIXAPP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <sys/stat.h>
#include "xap_App.h"
#include "xap_UnixDialogFactory.h"
#include "xap_Unix_TB_CFactory.h"

class AP_UnixToolbar_Icons;
class AV_View;
class XAP_UnixClipboard;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class ABI_EXPORT XAP_UnixApp : public XAP_App
{
public:
	XAP_UnixApp(const char* szAppName);
	virtual ~XAP_UnixApp();

	virtual const char * getDefaultEncoding () const
	  {
	    return "UTF-8" ;
	  }

	virtual bool					initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);
	void							shutdown();
	virtual XAP_Frame * 					newFrame() = 0;
	virtual void							reallyExit();

	virtual XAP_DialogFactory *				getDialogFactory();
	virtual XAP_Toolbar_ControlFactory *	getControlFactory();
	virtual const XAP_StringSet *			getStringSet() const = 0;
	virtual const char *					getAbiSuiteAppDir() const = 0;
	virtual const std::string&					getAbiSuiteAppUIDir() const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool							canPasteFromClipboard() = 0;
	void									migrate(const char *oldName, const char *newName, const char *path) const;
	virtual const char *					getUserPrivateDirectory() const;

	virtual void							setSelectionStatus(AV_View * pView) = 0;
	virtual void							clearSelection() = 0;
	virtual bool							getCurrentSelection(const char** formatList,
																void ** ppData, UT_uint32 * pLen,
																const char **pszFormatFound) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;

	virtual XAP_UnixClipboard * getClipboard () = 0;

	enum
	{
		GEOMETRY_FLAG_POS = 	1 << 0,
		GEOMETRY_FLAG_SIZE = 	1 << 1
	};

	struct windowGeometry
	{
		int x, y;
		UT_uint32 width, height;
		UT_uint32 flags;
	};

	virtual	void					setWinGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
												UT_uint32 flags);
	virtual	void					getWinGeometry(int * x, int * y, UT_uint32 * width, UT_uint32 * height,
												UT_uint32* flags);

	void							setTimeOfLastEvent(UT_uint32 eventTime);
	UT_uint32	   					getTimeOfLastEvent() const { return m_eventTime; };
    virtual XAP_App::BidiSupportType  theOSHasBidiSupport() const {return BIDI_SUPPORT_GUI;}
    char **                          getTmpFile(void)
	{ return &m_szTmpFile;}
	void                            removeTmpFile(void);
protected:
	void							_setAbiSuiteLibDir();

	AP_UnixDialogFactory			m_dialogFactory;
	AP_UnixToolbar_ControlFactory	m_controlFactory;

	windowGeometry			m_geometry;
	UT_uint32					m_eventTime; // e->time field of a recent X event
										 // (we use this to sync clipboard
										 // operations with the server).

    char *                     m_szTmpFile;
};

#endif /* XAP_UNIXAPP_H */
