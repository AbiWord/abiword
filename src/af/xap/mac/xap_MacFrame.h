/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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


#ifndef XAP_MACFRAME_H
#define XAP_MACFRAME_H

#include "xap_Frame.h"
#include "ut_vector.h"
#include "xap_MacDialogFactory.h"
class XAP_MacApp;
class ev_MacKeyboard;
class EV_MacMouse;
class EV_MacMenu;

/*****************************************************************
******************************************************************
** This file defines the Mac-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** Mac-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class XAP_MacFrame : public XAP_Frame
{
public:
	XAP_MacFrame(XAP_MacApp * app);
	XAP_MacFrame(XAP_MacFrame * f);
	virtual ~XAP_MacFrame(void);

	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Bool				loadDocument(const char * szFilename);
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);
	virtual UT_Bool				openURL(const char * szURL);

	virtual XAP_DialogFactory *	getDialogFactory(void);
	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);

protected:
	WindowPtr theWP;
	Rect theBounds;
};

#endif /* XAP_MACFRAME_H */
