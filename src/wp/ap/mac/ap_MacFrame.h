/* AbiWord
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

#ifndef AP_MACFRAME_H
#define AP_MACFRAME_H

#include "xap_MacFrame.h"

/*****************************************************************/

class AP_MacFrame : public XAP_MacFrame
{
public:
	AP_MacFrame(XAP_MacApp * app);
	AP_MacFrame(AP_MacFrame * f);
	virtual ~AP_MacFrame(void);

	virtual UT_Bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Bool				loadDocument(const char * szFilename, int ieft);
	virtual UT_Bool				initFrameData(void);
	virtual void				killFrameData(void);
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);

	virtual XAP_DialogFactory *	getDialogFactory(void);
	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);

	virtual void 				setStatusMessage(const char * szMsg);

};

#endif /* AP_MACFRAME_H */
