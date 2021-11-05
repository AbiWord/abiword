/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2019 Hubert Figuière
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


#include <gtk/gtk.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "ap_UnixLeftRuler.h"
#include "gr_UnixCairoGraphics.h"
#include "fv_View.h"

/*****************************************************************/

AP_UnixLeftRuler::AP_UnixLeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame),
	  AP_UnixRuler(pFrame)
{
}

AP_UnixLeftRuler::~AP_UnixLeftRuler(void)
{
	_aboutToDestroy(m_pFrame);
	DELETEP(m_pG);
}

GtkWidget * AP_UnixLeftRuler::createWidget(void)
{
	UT_ASSERT(!m_pG);
	return _createWidget(s_iFixedWidth, -1);
}

void AP_UnixLeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);
	_setView(pView, static_cast<GR_UnixCairoGraphics*>(m_pG));
}

void AP_UnixLeftRuler::_finishMotionEvent(UT_uint32, UT_uint32)
{
}
