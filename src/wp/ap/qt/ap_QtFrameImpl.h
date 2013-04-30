/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2012 Hubert Figuiere
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


#ifndef __AP_QT_FRAME_IMPL_H_
#define __AP_QT_FRAME_IMPL_H_

#include "xap_QtFrameImpl.h"

class AP_QtFrame;

class AP_QtFrameImpl
  : public XAP_QtFrameImpl
{
public:
	AP_QtFrameImpl(AP_QtFrame *pQtFrame);
	virtual ~AP_QtFrameImpl();
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame);

	virtual void _createWindow();

protected:
	virtual void _hideMenuScroll(bool bHideMenuScroll);
	virtual void _refillToolbarsInFrameData();
};

#endif
