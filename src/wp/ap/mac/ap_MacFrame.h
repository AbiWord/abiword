/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef AP_MACFRAME_H
#define AP_MACFRAME_H

#include "xap_MacFrame.h"

#include "ie_types.h"

/*****************************************************************/

class AP_MacFrame : public XAP_MacFrame
{
public:
	AP_MacFrame(XAP_MacApp * app);
	AP_MacFrame(AP_MacFrame * f);
	virtual ~AP_MacFrame(void);

	virtual bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Error			loadDocument(const char * szFilename, int ieft);
	virtual UT_Error                        loadDocument(const char * szFilename, int ieft, bool createNew);
	virtual bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);

	virtual void                            toggleTopRuler(bool bRulerOn);
	virtual void                            toggleLeftRuler(bool bRulerOn);

	virtual void 				setStatusMessage(const char * szMsg);
protected:
	UT_Error   				_showDocument(UT_uint32 iZoom=100);
	UT_Error   				_loadDocument(const char * szFilename, IEFileType ieft);
       	UT_Error				_replaceDocument(AD_Document * pDoc);
    	static void				_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void				_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
    virtual void						_createStatusBar(void);
};

#endif /* AP_MACFRAME_H */
