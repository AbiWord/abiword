/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

#include "xap_UnixFrame.h"

class AP_UnixStatusBar;

class AP_UnixFrame : public XAP_UnixFrame
{
public:
	AP_UnixFrame(XAP_UnixApp * app);
	AP_UnixFrame(AP_UnixFrame * f);
	virtual ~AP_UnixFrame(void);
	
	virtual UT_Bool     initialize(void);
	virtual XAP_Frame*  cloneFrame(void);
	virtual UT_Error     loadDocument(const char * szFilename, int fileType);
	virtual UT_Bool     initFrameData(void);
	virtual void        killFrameData(void);
	virtual void        setXScrollRange(void);
	virtual void        setYScrollRange(void);
	virtual void        translateDocumentToScreen(UT_sint32&, UT_sint32&);
	virtual void		toggleRuler(UT_Bool bRulerOn);	
protected:
	virtual GtkWidget*  _createDocumentWindow(void);
	virtual void        setStatusMessage(const char *);
	virtual GtkWidget * _createStatusBarWindow(void);
    virtual void				_setWindowIcon(void);
	GtkWidget* m_dArea;
	AP_UnixStatusBar* m_pUnixStatusBar;
};

#endif // AP_UNIXFRAME_H
