/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2019-2021 Hubert Figuière
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

#pragma once

// Common utilities for the left and top rulers.

#include "ut_types.h"
#include "gr_Graphics.h"
#include "ev_EditBits.h"
#include "xap_CustomWidget.h"

class ABI_EXPORT ap_RulerTicks
{
public:
	ap_RulerTicks(GR_Graphics * pG, UT_Dimension dim);
	UT_sint32 snapPixelToGrid(UT_sint32 dist);
	double scalePixelDistanceToUnits(UT_sint32 dist);

	GR_Graphics *	m_pG;


	UT_uint32		tickUnit;
	UT_uint32		tickUnitScale;
	UT_uint32		tickLong;
	UT_uint32		tickLabel;
	UT_uint32		tickScale;

	UT_uint32		dragDelta;

	UT_Dimension	dimType;
	double			dBasicUnit;
};

class AP_Ruler : virtual public XAP_CustomWidget
{
public:
    virtual ~AP_Ruler() {}
	virtual void setHeight(UT_uint32 iHeight) = 0;
	virtual void setWidth(UT_uint32 iWidth) = 0;
	virtual XAP_Frame* getFrame() const = 0;
	virtual void mouseMotion(EV_EditModifierState ems, UT_sint32 x, UT_sint32 y) = 0;
	virtual void mousePress(EV_EditModifierState ems, EV_EditMouseButton emb, UT_uint32 x, UT_uint32 y) = 0;
	virtual void mouseRelease(EV_EditModifierState ems, EV_EditMouseButton emb, UT_sint32 x, UT_sint32 y) = 0;
	virtual void _refreshView(void) = 0;
};
