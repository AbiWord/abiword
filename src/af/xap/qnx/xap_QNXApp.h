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


#ifndef XAP_QNXAPP_H
#define XAP_QNXAPP_H

#include "xap_App.h"
#include "xap_QNXDialogFactory.h"
#include "xap_QNX_TB_CFactory.h"
#include "xap_Strings.h"

class XAP_Args;
class AP_QNXToolbar_Icons;
class AV_View;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class XAP_QNXApp : public XAP_App
{
public:
	XAP_QNXApp(XAP_Args * pArgs, const char * szAppName);
	virtual ~XAP_QNXApp(void);

	virtual const char * getDefaultEncoding () const
	  {
	    return "UTF-8" ;
	  }

	virtual bool							initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);
	virtual XAP_Frame * 					newFrame(void) = 0;
	virtual void							reallyExit(void);

	virtual XAP_DialogFactory *				getDialogFactory(void);
	virtual XAP_Toolbar_ControlFactory *	getControlFactory(void);
	virtual const XAP_StringSet *			getStringSet(void) const = 0;
	virtual const char *					getAbiSuiteAppDir(void) const = 0;
	virtual void							copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true) = 0;
	virtual void							pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true) = 0;
	virtual bool							canPasteFromClipboard(void) = 0;
	virtual void							cacheCurrentSelection(AV_View *) = 0;
	virtual const char *					getUserPrivateDirectory(void);

  virtual XAP_App::BidiSupportType  theOSHasBidiSupport() const {return BIDI_SUPPORT_GUI;}

	void *					getFontManager(void);

	UT_sint32 makeDirectory(const char *, int) const;

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

	
	virtual	void					setWinGeometry(int x, int y, unsigned int width, unsigned int height,
																		UT_uint32 flags);
	virtual	void					getWinGeometry(int * x, int * y, unsigned int * width, unsigned int * height,
																		UT_uint32 * flags);
	
	void							setTimeOfLastEvent(unsigned int eventTime);
	unsigned int					getTimeOfLastEvent(void) const { return m_eventTime; };
	
protected:
	bool							_loadFonts(void);
	void							_setAbiSuiteLibDir();

	AP_QNXToolbar_Icons *			m_pQNXToolbarIcons;
	AP_QNXDialogFactory			m_dialogFactory;
	AP_QNXToolbar_ControlFactory	m_controlFactory;
	void *			m_fontManager;

	struct windowGeometry			m_geometry;
	unsigned int			m_eventTime; // e->time field of a recent X event
										 // (we use this to sync clipboard
										 // operations with the server).
};

#endif /* XAP_QNXAPP_H */
