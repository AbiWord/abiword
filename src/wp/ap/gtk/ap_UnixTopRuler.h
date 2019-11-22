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

#ifndef AP_UNIXTOPRULER_H
#define AP_UNIXTOPRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "ut_types.h"
#include "ap_TopRuler.h"
#include "ap_UnixRuler.h"

class XAP_Frame;

/*****************************************************************/

class AP_UnixTopRuler :
	public AP_TopRuler,
	public AP_UnixRuler
{
public:
	AP_UnixTopRuler(XAP_Frame * pFrame);
	virtual ~AP_UnixTopRuler(void);

	GtkWidget *		createWidget(void);
	virtual void setView(AV_View * pView) override;

protected:
	virtual XAP_Frame* _getFrame() const override
		{ return m_pFrame; }
	virtual GR_Graphics* _getGraphics() const override
		{ return m_pG; }
	virtual void _setGraphics(GR_Graphics* pG) override
		{ m_pG = pG; }
	virtual void _finishMotionEvent(UT_uint32 x, UT_uint32 y) override;
};

#endif /* AP_UNIXTOPRULER_H */
